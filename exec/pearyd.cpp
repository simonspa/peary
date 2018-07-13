/// \brief  Network daemon to setup and control peary devices.
/// \date   2018-07-05
/// \author Moritz Kiehn <msmk@cern.ch>

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using caribou::caribouDevice;
using caribou::caribouDeviceMgr;
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
// terminate with an optional error message
void terminate_failure(std::string err) {
  LOG(FATAL) << err;
  cleanup();
  std::exit(EXIT_FAILURE);
}
// terminate with an error message based on the current errno value
void terminate_errno() {
  LOG(FATAL) << "Error: " << std::strerror(errno);
  cleanup();
  std::exit(EXIT_FAILURE);
}

void termination_handler(int signal) {
  LOG(INFO) << "Caught user signal '" << signal << "', ending application";
  // ctrl-c is the regular way to close the application and not an error
  terminate_success();
}

// -----------------------------------------------------------------------------
// message framing over streaming connection

/// Read a fixed number of bytes into the buffer.
///
/// \returns true   on success
/// \returns false  upon disconnect of the client
///
/// \warning The buffer **must** be big enough to receive the requested data
bool read_fixed_into(int fd, size_t length, void* buffer)
{
  uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer);
  // keep-alive packages could have zero length payload
  if (length == 0) {
    return true;
  }
  do {
    ssize_t ret = read(fd, bytes, length);
    // unexpected client connection reset is not fatal
    if ((ret == -1) and (errno == ECONNRESET)) {
      LOG(ERROR) << "Connection reset by client during read";
      return false;
    }
    // any other error is fatal
    if (ret == -1) {
      terminate_errno();
    }
    // regular connection closure
    if (ret == 0) {
      return false;
    }
    length -= ret;
    bytes += ret;
  } while (0 < length);
  return true;
}

/// Read a length-encoded message into the buffer.
///
/// \returns true   on success
/// \returns false  upon disconnect of the client
bool read_msg_into(int fd, std::vector<uint8_t>& buffer)
{
  uint32_t len;

  // read message length
  if (!read_fixed_into(fd, 4, &len)) {
    // client disconnected
    return false;
  }
  // decode message length
  len = ntohl(len);
  LOG(DEBUG) << "Request message length=" << len;

  // read message content
  buffer.resize(len);
  if (!read_fixed_into(fd, len, buffer.data())) {
    LOG(ERROR) << "Client disconnected during request";
    buffer.clear();
    return false;
  }
  return true;
}

/// Write a length-encoded message using multiple buffers as input
///
/// \returns true   on success
/// \returns false  upon disconnect of the client
///
/// Each buffer must have a `.data()` and `.size()` member fuctions.
template <typename... Buffers>
bool write_msg(int fd, Buffers&&... buffers)
{
  constexpr size_t n = sizeof...(Buffers);

  // fill scatter/gather lists. first element is message length
  std::array<struct iovec, 1 + n> iov = {{
    { nullptr, 0 },
    { ((void*)buffers.data()), (buffers.size() * sizeof(*buffers.data())) }...
  }};

  // compute and check total message length
  size_t length = 0;
  for (size_t i = 1; i < (1 + n); ++i) {
    length += iov[i].iov_len;
  }
  if (UINT32_MAX < length) {
    terminate_failure("Reply message length > 2^32 - 1");
  }
  LOG(DEBUG) << "Reply message nbuffers=" << n << " total_length=" << length;

  // encoded message length
  uint32_t encoded_length = htonl(static_cast<uint32_t>(length));
  iov[0].iov_base = &encoded_length;
  iov[0].iov_len = 4;

  // write all message parts in a single step
  ssize_t ret = writev(fd, iov.data(), iov.size());
  // unexpected client connection reset is not fatal
  if ((ret == -1) and (errno == ECONNRESET)) {
    LOG(ERROR) << "Connection reset by client during write";
    return false;
  }
  // any other error is fatal
  if(ret == -1) {
    terminate_errno();
  }
  if (static_cast<size_t>(ret) != (4 + length)) {
    terminate_failure("Could not write full message");
  }
  return true;
}

// -----------------------------------------------------------------------------
// reply message encoding

enum ReplyError : uint8_t {
  // 0 is not part of this enum since it is not an error
  MalformedRequest = 1,
  UnknownCommand = 2,
};

struct ReplyBuffer {
  std::array<uint8_t, 5> header;
  std::string payload;

  void reset() {
    header.fill(0);
    payload.clear();
  }
  void set_sequence_encoded(const void* buf) {
      std::memcpy(header.data(), buf, 4);
  }
  void set_sequence(uint32_t seq) {
    *reinterpret_cast<uint32_t*>(header.data()) = htonl(seq);
  }
  void set_success() { header[4] = 0; }
  void set_error(ReplyError err) { header[4] = err; }
};

/// Split string into substrings using spaces as separator.
///
/// Each part is stripped of leading/following whitespace as well.
std::vector<std::string> split_by_whitespace(const char* txt, size_t len)
{
  std::vector<std::string> parts;

  constexpr char sep = ' ';
  while (0 < len) {
    // search first occurence of a non-separator byte
    if (*txt == sep) {
      txt += 1;
      len -= 1;
      continue;
    }
    const char* start = txt;
    // seach next occurence of a separator byte
    while (0 < len) {
      if (*txt == sep) {
        continue;
      }
      txt += 1;
      len -= 1;
    }
    // select substring
    parts.emplace_back(start, txt - start);
    LOG(DEBUG) << "split part '" << parts.back() << "'";
  }
  return parts;
}

// -----------------------------------------------------------------------------
// per-device commands

void do_device_command(const std::string& cmd,
                       const std::string& args,
                       caribouDeviceMgr& mgr,
                       ReplyBuffer& reply)
{
  // TODO add functionality
  // expected command format: <device_number>.command
  // 1. extract device number and get device
  // 2a. run custom functions for fixed functionality
  // 3a. reasonably encode result
  // 2b. run dispatcher function for everything else
  // 3b. dispatcher functions return string, use as payload

  reply.set_error(UnknownCommand);
  reply.payload = "Device commands not implemented";
}

// -----------------------------------------------------------------------------
// global commands

void do_hello(ReplyBuffer& reply) {
  reply.payload = "1"; // protocol version
  reply.set_success();
}

void do_add_device(const std::string& args, caribouDeviceMgr& mgr, ReplyBuffer& reply) {
  // TODO how to handle configuration
  caribou::Configuration cfg;

  size_t idx = mgr.addDevice(args, cfg);
  reply.payload = std::to_string(idx);
  reply.set_success();
}

void do_list_devices(caribouDeviceMgr& mgr, ReplyBuffer& reply) {
  reply.payload.clear();
  size_t idx = 0;
  for(caribouDevice* dev : mgr.getDevices()) {
    // not sure if the vector can contain nullptr?
    if(dev) {
      if(!reply.payload.empty()) {
        reply.payload.push_back('\n');
      }
      reply.payload.append(std::to_string(idx));
    }
    idx += 1;
  }
  reply.set_success();
}

// -----------------------------------------------------------------------------
// request/reply handling

void process_request(caribouDeviceMgr& mgr, const std::vector<uint8_t>& request, ReplyBuffer& reply) {

  // request **must** contain at least the sequence number
  if (request.size() < 4) {
    LOG(ERROR) << "Received malformed request";
    // no request sequence number is available
    reply.set_sequence(0);
    reply.set_error(MalformedRequest);
    reply.payload = "Malformed request";
    return;
  }
  // empty request is a keep-alive that returns no data
  if (request.size() == 4) {
    LOG(DEBUG) << "Received keep-alive";
    reply.set_sequence_encoded(request.data());
    reply.set_success();
    reply.payload.clear();
    return;
  }

  // reply **must** always contain the same sequence number as the request
  // is the same regardless of executed command
  reply.set_sequence_encoded(request.data());

  // split request into command and arguments
  const char* cmd = reinterpret_cast<const char*>(request.data() + 4);
  const char* args = cmd;
  size_t len_args = request.size() - 4;
  size_t len_cmd = 0;
  // find first space to detect end of command
  for(; (0 < len_args) and (*args != ' '); ++args, --len_args, ++len_cmd) {}
  // find next non-space to detect beginning of arguments
  for(; (0 < len_args) and (*args == ' '); ++args, --len_args) {}

  LOG(DEBUG) << "Received command '" << std::string(cmd, len_cmd) << "'";
  LOG(DEBUG) << "Received arguments '" << std::string(args, len_args) << "'";

  // select commands
  if (strncmp(cmd, "hello", len_cmd) == 0 ) {
    do_hello(reply);
  } else if (strncmp(cmd, "add_device", len_cmd) == 0) {
    do_add_device({args, len_args}, mgr, reply);
  } else if (strncmp(cmd, "list_devices", len_cmd) == 0) {
    do_list_devices(mgr, reply);
  } else if (strncmp(cmd, "device.", len_cmd) == 0) {
    // only need to hand over device number and device command
    do_device_command({cmd + 7, len_cmd - 7}, {args, len_args}, mgr, reply);
  } else {
    // everything else gives an error
    reply.set_error(UnknownCommand);
    reply.payload = "Unknown command: ";
    reply.payload.append(cmd, len_cmd);
  }
}

// -----------------------------------------------------------------------------
// application loop

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
      std::string msg;
      msg += "Unrecognized option '";
      msg += argv[i];
      msg += '\'';
      terminate_failure(std::move(msg));
    }
  }
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
    terminate_failure("Invalid verbosity level '" + arg_verbosity + "'");
  }

  // setup peary device manager
  caribouDeviceMgr mgr;

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

  // client loop
  // only a single client at any time, but multiple connections in series.
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
      terminate_failure("Inconsistent sockaddr size");
    }
    LOG(INFO) << "Client connected from "
              << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port);

    // reusable buffers
    std::vector<uint8_t> request_buffer;
    ReplyBuffer reply_buffer;

    // request/reply loop until client disconnects.
    while(true) {

      if(!read_msg_into(client_fd, request_buffer)) {
        break;
      }

      process_request(mgr, request_buffer, reply_buffer);

      if (!write_msg(client_fd, reply_buffer.header, reply_buffer.payload)) {
        break;
      }
    }

    close(client_fd);
    client_fd = -1;
    LOG(INFO) << "Client disconnected";
  }

  cleanup();
  return EXIT_SUCCESS;
}
