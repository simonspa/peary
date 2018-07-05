/// \brief  Network daemon to setup and control peary devices.
/// \date   2018-07-05
/// \author Moritz Kiehn <msmk@cern.ch>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <regex>
#include <vector>

#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using caribou::Log;
using caribou::LogLevel;

// global variables to allow cleanup in signal handler
int server_fd = -1;
int client_fd = -1;

// close any connections and flush logs
void cleanup() {
  if (client_fd != -1) {
    close(client_fd);
    client_fd = -1;
  }
  if (server_fd != -1) {
    close(server_fd);
    server_fd = -1;
  }
  Log::finish();
}

// clean up state and terminate application
void terminate_success() {
  cleanup();
  std::exit(EXIT_SUCCESS);
}
void terminate_failure() {
  cleanup();
  std::exit(EXIT_FAILURE);
}
// terminate with an error based on the current errno value
void terminate_errno() {
  LOG(ERROR) << "Error: " << std::strerror(errno) << '\n';
  cleanup();
  std::exit(EXIT_FAILURE);
}

void termination_handler(int signal) {
  LOG(INFO) << "Caught user signal '" << signal << "', ending application";
  // ctrl-c is the regular way to close the application and not an error
  terminate_success();
}

void show_help() {
  std::cout << "usage: pearyd [options]\n";
  std::cout << "\n";
  std::cout << "options:\n";
  std::cout << "  -h            show this help message\n";
  std::cout << "  -p port       server listen port, default 12345\n";
  std::cout << "  -v verbosity  verbosity level, default INFO\n";
  std::cout << std::flush;
}

// parse arguments and terminate on error
void parse_args(int argc, char* argv[], uint16_t& port, std::string& verbosity)
{
  // set default values
  port = 12345;
  verbosity = "INFO";

  for(int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-h")) {
      show_help();
      terminate_success();
    } else if(!strcmp(argv[i], "-p") and ((i + 1) < argc)) {
      port = std::stoul(argv[++i]);
    } else if(!strcmp(argv[i], "-v") and ((i + 1) < argc)) {
      verbosity = argv[++i];
    } else {
      std::cerr << "Unrecognized option '" << argv[i] << '\'' << std::endl;
      terminate_failure();
    }
  }
}

/// Read a fixed number of bytes into the buffer.
///
/// \returns true   on sucess
/// \returns false  upon disconnect of the client
///
/// \warning The buffer **must** be big enough to receive the requested data
bool read_fixed_into(int fd, uint32_t length, void* buffer)
{
  uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer);
  // keep-alive packages could have zero length payload
  if (length == 0) {
    return true;
  }
  do {
    ssize_t ret = read(fd, bytes, length);
    if (ret < 0) {
      terminate_errno();
    }
    if (ret == 0) {
      return false;
    }
    length -= ret;
    bytes += ret;
  } while (0 < length);
  return true;
}

/// Decode 8 byte header into sequence number and message length.
void decode_header(const uint8_t* buffer, uint32_t& sequence, uint32_t& length)
{
  sequence = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 0));
  length = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 4));
}

// Encode sequence number and message length into 8 byte header.
void encode_header(uint32_t sequence, uint32_t length, uint8_t* buffer)
{
  *reinterpret_cast<uint32_t*>(buffer + 0) = htonl(sequence);
  *reinterpret_cast<uint32_t*>(buffer + 4) = htonl(length);
}

// command handling

void process_command(const std::vector<uint8_t>& request, std::string& reply)
{
  const char* txt = reinterpret_cast<const char*>(request.data());
  size_t len = request.size();

  if (strncmp(txt, "hello", len) == 0) {
    reply = "ok 1";
  } else if (strncmp(txt, "add_device", len) == 0) {
    reply = "error not implemented";
  } else if (strncmp(txt, "list_devices", len) == 0) {
    reply = "ok";
  } else if (strncmp(txt, "device", len) == 0) {
    reply = "error not implemented";
  } else {
    reply = "error unknown command";
  }
}

std::string command_device(caribou::caribouDevice& dev,
                           const std::string& command,
                           const std::vector<std::string>& arguments)
{
  return "not implemented";
}

int main(int argc, char* argv[]) {
  // log to std::cout by default
  Log::addStream(std::cout);

  // handle ctrl-c
  struct sigaction signal_handler;
  signal_handler.sa_handler = termination_handler;
  sigemptyset(&signal_handler.sa_mask);
  signal_handler.sa_flags = 0;
  sigaction(SIGINT, &signal_handler, nullptr);

  // handle options
  uint16_t arg_port;
  std::string arg_verbosity;
  parse_args(argc, argv, arg_port, arg_verbosity);

  // update log level
  try {
    LogLevel log_level = Log::getLevelFromString(arg_verbosity);
    Log::setReportingLevel(log_level);
  } catch(std::invalid_argument& e) {
    LOG(ERROR) << "Invalid verbosity level '" << arg_verbosity << "'";
    terminate_failure();
  }

  // TODO setup peary manager, etc...

  // setup server
  struct sockaddr_in server_addr;
  socklen_t server_addrlen = sizeof(server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(arg_port);

  // create tcp/ip socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    terminate_errno();
  }
  // avoid error when socket is reused. see also:
  // http://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
  int yes = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    terminate_errno();
  }
  // bind to listen address
  if (bind(server_fd, (struct sockaddr*)&server_addr, server_addrlen) == -1) {
    terminate_errno();
  }
  // start listening for connections, allow only one client at a time
  if (listen(server_fd, 1) == -1) {
    terminate_errno();
  }
  LOG(INFO) << "Listening for connections on "
            << inet_ntoa(server_addr.sin_addr)
            << ":" << ntohs(server_addr.sin_port);

  // we serve only a single client at any given time, but allow multiple
  // serial connections one after one another.
  while (true) {
    // wait for a client to connect
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addrlen);
    if(client_fd == -1) {
      LOG(WARNING) << "Failure to accept connection: " << std::strerror(errno);
      // try again accepting client connections
      continue;
    }
    if(client_addrlen != sizeof(client_addr)) {
      LOG(ERROR) << "Inconsistent sockaddr size";
      terminate_failure();
    }
    LOG(INFO) << "Client connected from "
              << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port);

    // reusable buffers
    std::array<uint8_t, 8> header;
    std::vector<uint8_t> request;
    std::string reply;

    while(true) {
      // read request header
      uint32_t sequence, length;
      if(!read_fixed_into(client_fd, 8, header.data())) {
        LOG(INFO) << "Client disconnected";
        // allow next client to connect
        break;
      }
      decode_header(header.data(), sequence, length);
      LOG(DEBUG) << "Request header sequence=" << sequence << " length=" << length;
      // read request message
      request.resize(length);
      if(!read_fixed_into(client_fd, length, (void*)request.data())) {
        LOG(ERROR) << "Client disconnected during request";
        break;
      }

      process_command(request, reply);

      if(UINT32_MAX < reply.size()) {
        LOG(ERROR) << "Reply message is larger than 2^32 bytes";
        terminate_failure();
      }
      // send reply header with the same sequence number
      LOG(DEBUG) << "Request header sequence=" << sequence << " length=" << reply.size();
      encode_header(sequence, reply.size(), header.data());
      if(write(client_fd, header.data(), 8) == -1) {
        terminate_errno();
      }
      if(write(client_fd, reply.data(), reply.size()) == -1) {
        terminate_errno();
      }
    }

    close(client_fd);
    client_fd = -1;
  }

  cleanup();
  return EXIT_SUCCESS;
}
