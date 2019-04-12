#ifndef CARIBOU_HAL_MEMORY_HPP
#define CARIBOU_HAL_MEMORY_HPP

#include <cstdint>
#include <fcntl.h>
#include <mutex>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <vector>

#include "InterfaceManager.hpp"
#include "datatypes.hpp"
#include "exceptions.hpp"
#include "Interface.hpp"

namespace caribou {

  class iface_mem : public Interface<memory_map, size_t, uint32_t> {

  private:
    // Default constructor: private (only created by InterfaceManager)
    //
    // It can throw DeviceException
    iface_mem(std::string const& device_path);

    virtual ~iface_mem();

    // Access to FPGA memory mapped registers
    int _memfd;

    // Buffer
    std::map<memory_map, void*> _mappedMemory;

    // Protects access to the bus
    std::mutex mutex;

    template <typename T> friend class caribouHAL;

  private:
    std::pair<size_t, uint32_t> write(const memory_map&, const std::pair<size_t, uint32_t>&);

    uint32_t readWord(const memory_map&, const size_t);
    std::vector<uint32_t> read(const memory_map&, const size_t, const unsigned int);

    void* mapMemory(const memory_map&);

    // Remove default constructor
    iface_mem() = delete;

    // only this function can create the interface
    friend iface_mem& InterfaceManager::getInterface<iface_mem>(std::string const&);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_MEMORY_HPP */
