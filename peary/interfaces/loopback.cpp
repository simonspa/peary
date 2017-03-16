/**
 * Caribou Loopback interface class implementation
 */

#include "utils.hpp"
#include "log.hpp"


#include <loopback.hpp>

using namespace caribou;

iface_loopback::iface_loopback(std::string const & device_path) : Interface( device_path ) {
  LOG(logINTERFACE) << "Opened LOOPBACK device at " << device_path;
}

iface_loopback::~iface_loopback() {
  LOG(logINTERFACE) << "Closed LOOPBACK device at " << devicePath;
}

uint8_t iface_loopback::write(const uint8_t& address, const uint8_t& data ){
  std::lock_guard<std::mutex> lock(mutex);
  
  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath
		    <<") : Writing data \"" << static_cast<int>(data)
		    <<  "\" at address " << to_hex_string(address) << std::dec;

  return data;
}

std::vector<uint8_t> iface_loopback::write(const uint8_t& address, const std::vector<uint8_t>& data){
  std::lock_guard<std::mutex> lock(mutex);
  
  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath
		    <<") : Writing data \"" << listVector(data)
		    <<  "\" at address " << to_hex_string(address) << std::dec;

  return data;
}

std::pair<uint8_t, uint8_t> iface_loopback::write(const uint8_t& address,const std::pair<uint8_t, uint8_t> & data){

  std::lock_guard<std::mutex> lock(mutex);
  
  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath
		    <<") : Writing data \"" << static_cast<int>(data.second) << "\" to register "
		    << static_cast<int>(data.first)
		    <<  " at address " << to_hex_string(address) << std::dec;

  return data;
}
  
std::vector<uint8_t> iface_loopback::write(const uint8_t& address, const uint8_t & reg, const std::vector< uint8_t > & data){

  std::lock_guard<std::mutex> lock(mutex);
    
  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath <<") : Writing block data: \""
		    << listVector(data) << "\"";
  LOG(logINTERFACE) << " to register " << static_cast<int>(reg)
		    << " at address " << to_hex_string(address) << std::dec;
  return data;
}

std::vector< std::pair<uint8_t, uint8_t> > iface_loopback::write(const uint8_t& address, const std::vector< std::pair<uint8_t, uint8_t> >& data){
  
  std::lock_guard<std::mutex> lock(mutex);
  
  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath
		    <<") : Writing block data to registers:";
  for(auto i : data)
    LOG(logINTERFACE) << static_cast<int>(i.first) << " | " << static_cast<int>(i.second);
  LOG(logINTERFACE) <<  "at address " << to_hex_string(address) << std::dec;

  return data;
}

std::vector<uint8_t> iface_loopback::read(const uint8_t& address, const unsigned int length) {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<uint8_t> data;

  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath <<") address " << to_hex_string(address) << ": Read data  - returning address." << std::dec;

  for(unsigned int i = 0; i < length; i++)
    data.push_back(address);
  return data;
}

std::vector<uint8_t> iface_loopback::read(const uint8_t& address, const uint8_t reg, const unsigned int length){
  
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<uint8_t> data;

  LOG(logINTERFACE) << std::hex << "LOOPBACK (" << devicePath <<") address " << to_hex_string(address) << ": Read data from register " << static_cast<int>(reg) << " - returning address." << std::dec;

  for(unsigned int i = 0; i < length; i++)
    data.push_back(address);
  return data;
}
