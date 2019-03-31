/**
 * Caribou Memory interface class emulator
 */

#include "log.hpp"
#include "utils.hpp"

#include "memory.hpp"

using namespace caribou;

iface_mem::iface_mem(std::string const& device_path) : Interface(device_path), _memfd(), _mappedMemory() {
  LOG(TRACE) << "Opened emulated memory device at " << device_path;
}

iface_mem::~iface_mem() {}

std::pair<size_t, uint32_t> iface_mem::write(const memory_map&, const std::pair<size_t, uint32_t>&) {
  LOG(TRACE) << "Writing to mapped memory at " << std::hex << mem.getBaseAddress() << ", offset " << dest.first << std::dec
             << ": " << dest.second;
  return std::pair<size_t, uint32_t>();
}

uint32_t iface_mem::read(const memory_map&, const size_t) {
  LOG(TRACE) << "Reading from mapped memory at " << std::hex << mem.getBaseAddress() << ", offset " << offset << std::dec;
  return 0;
}

std::vector<uint32_t> iface_mem::read(const memory_map& mem, const size_t offset, const unsigned int n) {
  std::vector<uint32_t> values;
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(this->read(mem, offset));
  }
  return values;
}
