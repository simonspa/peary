/**
 * Caribou Memory interface class implementation
 */

#include "log.hpp"
#include "utils.hpp"

#include "memory.hpp"

using namespace caribou;

iface_mem::iface_mem(std::string const& device_path) : Interface(device_path), _memfd(-1), _mappedMemory() {

  // Get access to FPGA memory mapped registers
  _memfd = open(device_path.c_str(), O_RDWR | O_SYNC);

  // If we still don't have one, something went wrong:
  if(_memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }
  LOG(TRACE) << "Opened memory device at " << device_path;
}

iface_mem::~iface_mem() {
  // Unmap all mapped memory pages:
  for(auto& mem : _mappedMemory) {
    LOG(TRACE) << "Unmapping memory at " << std::hex << mem.first.getBaseAddress() << std::dec;
    if(munmap(mem.second, mem.first.getSize()) == -1) {
      LOG(FATAL) << "Can't unmap memory from user space.";
    }
  }

  close(_memfd);
}

std::pair<size_t, uint32_t> iface_mem::write(const memory_map& mem, const std::pair<size_t, uint32_t>& dest) {
  LOG(TRACE) << "Writing to mapped memory at " << std::hex << mem.getBaseAddress() << ", offset " << dest.first << std::dec
             << ": " << dest.second;
  volatile uint32_t* reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(mapMemory(mem)) + mem.getOffset() + dest.first);
  *reg = dest.second;
  return std::pair<size_t, uint32_t>();
}

uint32_t iface_mem::readWord(const memory_map& mem, const size_t offset) {
  volatile uint32_t* reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(mapMemory(mem)) + mem.getOffset() + offset);
  uint32_t value = *reg;
  LOG(TRACE) << "Reading from mapped memory at " << std::hex << mem.getBaseAddress() << ", offset " << offset << std::dec
             << ": " << value;
  return value;
}

std::vector<uint32_t> iface_mem::read(const memory_map& mem, const size_t offset, const unsigned int n) {
  std::vector<uint32_t> values;
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(this->readWord(mem, offset));
  }
  return values;
}

void* iface_mem::mapMemory(const memory_map& page) {

  // Check if this memory page is already mapped and return the pointer:
  LOG(TRACE) << "Returning mapped memory at " << std::hex << page.getBaseAddress() << std::dec;
  try {
    return _mappedMemory.at(page);
  }
  // Otherwise newly map it and return the reference:
  catch(const std::out_of_range& oor) {
    LOG(TRACE) << "Memory was not yet mapped, mapping...";
    // Map one page of memory into user space such that the device is in that page, but it may not
    // be at the start of the page.
    void* map_base = mmap(0, page.getSize(), page.getFlags(), MAP_SHARED, _memfd, page.getBaseAddress() & ~page.getMask());
    if(map_base == (void*)-1) {
      throw DeviceException("Can't map the memory to user space.\n");
    }

    // get the address of the device in user space which will be an offset from the base
    // that was mapped as memory is mapped at the start of a page
    void* base_pointer =
      reinterpret_cast<void*>(reinterpret_cast<std::intptr_t>(map_base) + (page.getBaseAddress() & page.getMask()));

    // Store the mapped memory, so we can unmap it later:
    _mappedMemory[page] = base_pointer;
    return base_pointer;
  }
}
