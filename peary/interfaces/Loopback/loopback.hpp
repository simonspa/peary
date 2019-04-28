#ifndef CARIBOU_HAL_LOOPBACK_HPP
#define CARIBOU_HAL_LOOPBACK_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "Interface.hpp"
#include "InterfaceManager.hpp"
#include "exceptions.hpp"

namespace caribou {

  class iface_loopback : public Interface<uint8_t, uint8_t, uint8_t> {

  private:
    /* Default constructor: private (only created by InterfaceManager)
     *
     * Throws caribou::DeviceException in case no connection can be established
     */
    iface_loopback(std::string const& device_path);

    ~iface_loopback();

    // Protects access to the bus
    std::mutex mutex;

    template <typename T> friend class caribouHAL;

  private:
    uint8_t write(const uint8_t& address, const uint8_t& data);
    std::vector<uint8_t> write(const uint8_t& address, const std::vector<uint8_t>& data);
    std::pair<uint8_t, uint8_t> write(const uint8_t& address, const std::pair<uint8_t, uint8_t>& data);
    std::vector<uint8_t> write(const uint8_t& address, const uint8_t& reg, const std::vector<uint8_t>& data);
    std::vector<std::pair<uint8_t, uint8_t>> write(const uint8_t& address,
                                                   const std::vector<std::pair<uint8_t, uint8_t>>& data);
    std::vector<uint8_t> read(const uint8_t& address, const unsigned int length = 1);
    std::vector<uint8_t> read(const uint8_t& address, const uint8_t reg, const unsigned int length = 1);

    // Unused constructor
    iface_loopback() = delete;

    // only this function can create the interface
    friend iface_loopback& InterfaceManager::getInterface<iface_loopback>(std::string const&);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_LOOPBACK_HPP */
