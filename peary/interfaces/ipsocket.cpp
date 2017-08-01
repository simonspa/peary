/**
 * Caribou I2C interface class implementation
 */

#include <cerrno>
#include <cstring>
#include <sstream>

// OS socket support
#include <arpa/inet.h>
#include <sys/socket.h>

#include "log.hpp"
#include "utils.hpp"

#include "ipsocket.hpp"

#define BUFFERSIZE 4096

using namespace caribou;

iface_ipsocket::iface_ipsocket(std::string const& device_path) : Interface(device_path) {

  LOG(logINTERFACE) << "New IP sockets interface requested.";
  auto ip_address = split_ip_address(device_path);
  LOG(logINTERFACE) << "IP address: " << ip_address.first;
  LOG(logINTERFACE) << "      port: " << ip_address.second;

  // Configure Socket and address
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(ip_address.second);
  inet_aton(ip_address.first.c_str(), &(address.sin_addr));

  mysocket_ = socket(AF_INET, SOCK_STREAM, 0);

  std::stringstream ss;
  ss << inet_ntoa(address.sin_addr);

  LOG(logINTERFACE) << "Connecting to " << ss.str();
  int retval = ::connect(mysocket_, (struct sockaddr*)&address, sizeof(address));

  if(retval == 0) {
    LOG(logINTERFACE) << "Connection to server at " << ss.str() << " established";
  } else {
    LOG(logCRITICAL) << "Connection to server at " << ss.str() << " failed, errno " << errno;
    throw CommunicationError("Connection to server at " + ss.str() + " failed, errno " + std::to_string(errno));
  }
}

std::pair<std::string, uint32_t> iface_ipsocket::split_ip_address(std::string address) {

  std::pair<std::string, uint32_t> ip_and_port;

  std::istringstream ss(address);
  std::string token;
  size_t tokens = 0;

  while(std::getline(ss, token, ':')) {
    if(tokens == 0)
      ip_and_port.first = token;
    else if(tokens == 1)
      ip_and_port.second = std::stoi(token);
    tokens++;
  }

  if(tokens != 2)
    throw ConfigInvalid("Cannot retrieve port and IP address from provided device path: \"" + address + "\"");
  return ip_and_port;
}

iface_ipsocket::~iface_ipsocket() {
  // When finished, close the sockets
  close(mysocket_);
}

ipsocket_t iface_ipsocket::write(const ipsocket_port_t&, ipsocket_t& data) {
  std::lock_guard<std::mutex> lock(mutex);

  if(data.back() != '\n') {
    LOG(logINTERFACE) << "Add carriage return to command.";
    data += '\n';
  }

  LOG(logINTERFACE) << "Sending command \"" << data << "\"";
  int retval = send(mysocket_, data.c_str(), strlen(data.c_str()), 0);

  if(retval < 0) {
    LOG(logERROR) << "Command returned: " << retval << std::endl;
    throw CommunicationError("Server returned errno " + errno);
  }
  return std::string();
}

std::vector<ipsocket_t> iface_ipsocket::read(const ipsocket_port_t&, ipsocket_t& query, const unsigned int) {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<ipsocket_t> data;
  ipsocket_t dataword;

  // Send query command:
  write(0, query);

  int msglen = 0;
  char buffer[BUFFERSIZE];
  int msgs = 0;
  while(buffer[msglen - 1] != '\n') {
    LOG(logINTERFACE) << "Receiving buffer from socket...";
    // retrieve response:
    msglen = recv(mysocket_, buffer, BUFFERSIZE, 0);
    LOG(logINTERFACE) << "Received: \"" << std::string(buffer) << "\"";
    // Terminate the string:
    buffer[msglen] = '\0';
    dataword += std::string(buffer);
    // Reset buffer:
    buffer[0] = '\0';
    msgs++;
  }

  data.push_back(dataword);
  LOG(logINTERFACE) << "Received " << msgs << " buffers.";
  return data;
}
