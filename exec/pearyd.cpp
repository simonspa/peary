/// \brief  Network daemon to setup and control peary devices.
/// \date   2018-07-05
/// \author Moritz Kiehn <msmk@cern.ch>

#include <cstdint>
#include <cstdlib>
#include <cstring>
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
  if(client_fd != -1) {
    close(client_fd);
    client_fd = -1;
  }
  if(server_fd != -1) {
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
bool read_fixed_into(int fd, size_t length, void* buffer) {
  uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer);
  // keep-alive packages could have zero length payload
  if(length == 0) {
    return true;
  }
  do {
    ssize_t ret = read(fd, bytes, length);
    // unexpected client connection reset is not fatal
    if((ret == -1) and (errno == ECONNRESET)) {
      LOG(ERROR) << "Connection reset by client during read";
      return false;
    }
    // any other error is fatal
    if(ret == -1) {
      terminate_errno();
    }
    // regular connection closure
    if(ret == 0) {
      return false;
    }
    length -= ret;
    bytes += ret;
  } while(0 < length);
  return true;
}

/// Read a length-encoded message into the buffer.
///
/// \returns true   on success
/// \returns false  upon disconnect of the client
bool read_msg_into(int fd, std::vector<uint8_t>& buffer) {
  uint32_t len;

  // read message length
  if(!read_fixed_into(fd, 4, &len)) {
    // client disconnected
    return false;
  }
  // decode message length
  len = ntohl(len);
  LOG(DEBUG) << "Request message length=" << len;

  // read message content
  buffer.resize(len);
  if(!read_fixed_into(fd, len, buffer.data())) {
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
template <typename... Buffers> bool write_msg(int fd, Buffers&&... buffers) {
  constexpr size_t n = sizeof...(Buffers);

  // fill scatter/gather lists. first element is message length
  std::array<struct iovec, 1 + n> iov = {
    {{nullptr, 0}, {((void*)buffers.data()), (buffers.size() * sizeof(*buffers.data()))}...}};

  // compute and check total message length
  size_t length = 0;
  for(size_t i = 1; i < (1 + n); ++i) {
    length += iov[i].iov_len;
  }
  if(UINT32_MAX < length) {
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
  if((ret == -1) and (errno == ECONNRESET)) {
    LOG(ERROR) << "Connection reset by client during write";
    return false;
  }
  // any other error is fatal
  if(ret == -1) {
    terminate_errno();
  }
  if(static_cast<size_t>(ret) != (4 + length)) {
    terminate_failure("Could not write full message");
  }
  return true;
}

// -----------------------------------------------------------------------------
// common header and message handling

enum class Status : uint16_t {
  Ok = 0,
  // message-level errors
  MessageInvalid = 2,
  // command-level errors
  CommandUnknown = 16,
  CommandNotEnoughArguments = 17,
  CommandTooManyArguments = 18,
  CommandInvalidArgument = 19,
  CommandFailure = 20,
};

struct Header {
  std::array<uint16_t, 2> encoded;

  // direct compatibility w/ write_msg(...)
  const uint8_t* data() const { return reinterpret_cast<const uint8_t*>(encoded.data()); }
  size_t size() const { return 4u; }

  uint16_t sequence() const { return ntohs(encoded[0]); }
  Status status() const { return static_cast<Status>(ntohs(encoded[1])); }

  Header() = default;
  Header(const void* buffer) { std::memcpy(encoded.data(), buffer, 4); }
  void set_sequence(uint16_t sequence_) { encoded[0] = htons(sequence_); }
  void set_status(Status status_) { encoded[1] = htons(static_cast<uint16_t>(status_)); }
};

struct ReplyBuffer {
  Header header;
  std::string payload;

  void clear() {
    header.set_sequence(0);
    header.set_status(Status::Ok);
    payload.clear();
  }
  void set_sequence(uint16_t seq) { header.set_sequence(seq); }
  void set_success() { header.set_status(Status::Ok); }
  void set_status(Status status) { header.set_status(status); }
};

// -----------------------------------------------------------------------------
// string helpers

/// Split string into two parts using the first occurence of the separator.
///
/// \returns Length of the first part excluding the separator.
size_t split_once(const char* txt, size_t len, char separator) {
  size_t idx = 0;
  while((idx < len) && (txt[idx] != separator)) {
    idx += 1;
  }
  return idx;
}

/// Split string into substrings using spaces as separator.
///
/// Each part is stripped of leading/following whitespace as well.
std::vector<std::string> split(const char* txt, size_t len, char separator) {
  std::vector<std::string> parts;

  // empty input string should yield empty split result
  while(0 < len) {
    size_t part_len = split_once(txt, len, separator);
    parts.emplace_back(txt, part_len);
    // skip separator
    if(part_len < len) {
      part_len += 1;
    }
    txt += part_len;
    len -= part_len;
  }
  return parts;
}

// -----------------------------------------------------------------------------
// per-device commands

/// Check for the correct number of arguments and set reply status if necessary.
bool check_num_args(const std::vector<std::string>& args, size_t expected, ReplyBuffer& reply) {
  if(args.size() < expected) {
    reply.set_status(Status::CommandNotEnoughArguments);
    return false;
  }
  if(expected < args.size()) {
    reply.set_status(Status::CommandTooManyArguments);
    return false;
  }
  return true;
}

void do_device_list_registers(caribouDevice& device, ReplyBuffer& reply) {
  reply.payload.clear();
  for(const auto& reg : device.getRegisters()) {
    if(!reply.payload.empty()) {
      reply.payload.push_back('\n');
    }
    // only return the register names
    reply.payload.append(reg.first);
  }
  reply.set_success();
}

void do_device_get_register(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 1, reply)) {
    reply.payload = std::to_string(device.getRegister(args[0]));
    reply.set_success();
  }
}

void do_device_set_register(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 2, reply)) {
    device.setRegister(args[0], std::stoul(args[1]));
    reply.set_success();
  }
}

void do_device_get_current(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 1, reply)) {
    reply.payload = std::to_string(device.getCurrent(args[0]));
    reply.set_success();
  }
}

void do_device_set_current(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 3, reply)) {
    // third argument is polarity
    device.setCurrent(args[0], std::stod(args[1]), std::stoi(args[2]));
    reply.set_success();
  }
}

void do_device_get_voltage(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 1, reply)) {
    reply.payload = std::to_string(device.getCurrent(args[0]));
    reply.set_success();
  }
}

void do_device_set_voltage(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 3, reply)) {
    // third argument is current limit
    device.setVoltage(args[0], std::stod(args[1]), std::stod(args[2]));
    reply.set_success();
  }
}

void do_device_switch_on(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 1, reply)) {
    device.switchOn(args[0]);
    reply.set_success();
  }
}

void do_device_switch_off(caribouDevice& device, const std::vector<std::string>& args, ReplyBuffer& reply) {
  if(check_num_args(args, 1, reply)) {
    device.switchOff(args[0]);
    reply.set_success();
  }
}

void do_device(const std::string& cmd, const std::vector<std::string>& args, caribouDeviceMgr& mgr, ReplyBuffer& reply) {

  // parse command format: device.<device_id>.command
  auto cmds = split(cmd.data(), cmd.size(), '.');
  if((cmds.size() != 3) or (cmds[0] != "device")) {
    LOG(ERROR) << "Invalid device command '" << cmd << '\'';
    reply.set_status(Status::CommandUnknown);
    reply.payload = "Invalid device command";
    return;
  }

  // find corresponding device
  caribouDevice* device = nullptr;
  try {
    size_t device_id = std::stoul(cmds[1]);
    device = mgr.getDevice(device_id);
  } catch(const caribou::DeviceException& e) {
    LOG(ERROR) << "Invalid device identifier " << cmds[1];
    reply.set_status(Status::CommandFailure);
    reply.payload = "Invalid device identifier";
    return;
  }

  // reset status here
  reply.set_success();

  // execute per-device command
  const auto& device_cmd = cmds[2];
  if(device_cmd == "name") {
    reply.payload = device->getName();
  } else if(device_cmd == "firmware_version") {
    reply.payload = device->getFirmwareVersion();
  } else if(device_cmd == "power_on") {
    device->powerOn();
  } else if(device_cmd == "power_off") {
    device->powerOff();
  } else if(device_cmd == "reset") {
    device->reset();
  } else if(device_cmd == "configure") {
    device->configure();
  } else if(device_cmd == "daq_start") {
    device->daqStart();
  } else if(device_cmd == "daq_stop") {
    device->daqStop();
  } else if(device_cmd == "list_registers") {
    do_device_list_registers(*device, reply);
  } else if(device_cmd == "get_register") {
    do_device_get_register(*device, args, reply);
  } else if(device_cmd == "set_register") {
    do_device_set_register(*device, args, reply);
  } else if(device_cmd == "get_current") {
    do_device_get_current(*device, args, reply);
  } else if(device_cmd == "set_current") {
    do_device_set_current(*device, args, reply);
  } else if(device_cmd == "get_voltage") {
    do_device_get_voltage(*device, args, reply);
  } else if(device_cmd == "set_voltage") {
    do_device_set_voltage(*device, args, reply);
  } else if(device_cmd == "switch_on") {
    do_device_switch_on(*device, args, reply);
  } else if(device_cmd == "switch_off") {
    do_device_switch_off(*device, args, reply);
  } else {
    // try command w/ the dynamic dispatcher
    try {
      reply.payload = device->command(device_cmd, args);
    } catch(const caribou::ConfigInvalid& e) {
      LOG(ERROR) << "Unknown command '" << device_cmd << "' for device " << device->getName();
      reply.set_status(Status::CommandUnknown);
      reply.payload = e.what();
    }
  }
}

// -----------------------------------------------------------------------------
// global commands

void do_add_device(const std::vector<std::string>& args, caribouDeviceMgr& mgr, ReplyBuffer& reply) {
  // TODO how to handle configuration
  caribou::Configuration cfg;

  if(args.size() < 1) {
    reply.set_status(Status::CommandNotEnoughArguments);
  } else if(1 < args.size()) {
    reply.set_status(Status::CommandTooManyArguments);
  } else {
    size_t idx = mgr.addDevice(args.front(), cfg);
    reply.set_success();
    reply.payload = std::to_string(idx);
  }
}

void do_list_devices(caribouDeviceMgr& mgr, ReplyBuffer& reply) {
  reply.set_success();
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
}

void do_protocol_version(ReplyBuffer& reply) {
  reply.set_success();
  reply.payload = "1"; // protocol version
}

// -----------------------------------------------------------------------------
// request/reply handling

void process_request(caribouDeviceMgr& mgr, const std::vector<uint8_t>& request, ReplyBuffer& reply) {
  // request **must** contain at least the header
  if(request.size() < 4) {
    LOG(ERROR) << "Received malformed request, message is too small";
    // no request sequence number is available
    reply.set_sequence(0);
    reply.set_status(Status::MessageInvalid);
    reply.payload = "Message too small";
    return;
  }

  // unpack request header and payload
  Header request_header(request.data());
  const char* payload_data = reinterpret_cast<const char*>(request.data() + 4);
  size_t payload_len = request.size() - 4;

  // reply **must** always contain the request sequence number
  reply.clear();
  reply.set_sequence(request_header.sequence());

  if(request_header.status() != Status::Ok) {
    LOG(ERROR) << "Received malformed request, invalid status";
    reply.set_status(Status::MessageInvalid);
    reply.payload = "Status is not Ok";
    return;
  }
  // empty request is keep-alive that returns no data
  if(payload_len == 0) {
    LOG(INFO) << "Received keep-alive";
    reply.set_success();
    return;
  }

  // payload comprises a command and its arguments
  std::string cmd(payload_data, split_once(payload_data, payload_len, ' '));
  std::vector<std::string> args;
  // only split arguments if there are actually some available
  if(cmd.size() < payload_len) {
    size_t start_args = cmd.size() + 1; // ignore separator
    args = split(payload_data + start_args, payload_len - start_args, ' ');
  }

  LOG(DEBUG) << "Received command '" << cmd << "'";
  for(const auto& arg : args) {
    LOG(DEBUG) << "Received argument '" << arg << "'";
  }

  // execute commands
  if(cmd.find("device") == 0) {
    // per-device commands are handled separately
    do_device(cmd, args, mgr, reply);
  } else if(cmd == "add_device") {
    do_add_device(args, mgr, reply);
  } else if(cmd == "list_devices") {
    do_list_devices(mgr, reply);
  } else if(cmd == "protocol_version") {
    do_protocol_version(reply);
  } else {
    // everything else is an error
    reply.set_status(Status::CommandUnknown);
    reply.payload = "Unknown command: ";
    reply.payload += cmd;
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
void parse_args(int argc, char* argv[], uint16_t& port, std::string& verbosity) {
  // set default values
  port = 12345;
  verbosity = "INFO";

  for(int i = 1; i < argc; ++i) {
    if(!strcmp(argv[i], "-h")) {
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
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    terminate_errno();
  }
  // avoid error when socket is reused. see also:
  // http://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
  int yes = 1;
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    terminate_errno();
  }
  // bind to listen address
  if(bind(server_fd, (struct sockaddr*)&server_addr, server_addrlen) == -1) {
    terminate_errno();
  }
  // start listening for connections, allow only one client at a time
  if(listen(server_fd, 1) == -1) {
    terminate_errno();
  }
  LOG(INFO) << "Listening for connections on " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port);

  // client loop
  // only a single client at any time, but multiple connections in series.
  while(true) {
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
    LOG(INFO) << "Client connected from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);

    // reusable buffers
    std::vector<uint8_t> request_buffer;
    ReplyBuffer reply_buffer;

    // request/reply loop until client disconnects.
    while(true) {

      if(!read_msg_into(client_fd, request_buffer)) {
        break;
      }
      process_request(mgr, request_buffer, reply_buffer);
      if(!write_msg(client_fd, reply_buffer.header, reply_buffer.payload)) {
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
