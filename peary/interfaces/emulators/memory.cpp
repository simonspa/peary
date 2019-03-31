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
  return std::pair<size_t, uint32_t>();
}

uint32_t iface_mem::read(const memory_map&, const size_t) {
  return 0;
}

std::vector<uint32_t> iface_mem::read(const memory_map&, const size_t, const unsigned int) {
  std::vector<uint32_t> values;
  return values;
}
