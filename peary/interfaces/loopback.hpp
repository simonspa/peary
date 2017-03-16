#ifndef CARIBOU_HAL_LOOPBACK_HPP
#define CARIBOU_HAL_LOOPBACK_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <mutex>

#include "exceptions.hpp"
#include "interface_manager.hpp"
#include "interface.hpp"

namespace caribou {

  class iface_loopback : public Interface<uint8_t, uint8_t, uint8_t> {

  private:

    /* Default constructor: private (only created by interface_manager)
     *
     * Throws caribou::DeviceException in case no connection can be established
     */
    iface_loopback(std::string const & device_path);
    
    ~iface_loopback();
    
    // Protects access to the bus
    std::mutex mutex;

  private:
    uint8_t write(const uint8_t& address, const uint8_t& data );
    std::vector<uint8_t> write(const uint8_t& address, const std::vector<uint8_t>& data );
    std::pair<uint8_t, uint8_t> write(const uint8_t& address, const std::pair<uint8_t, uint8_t> & data);
    std::vector<uint8_t> write(const uint8_t& address, const uint8_t & reg, const std::vector< uint8_t > & data);
    std::vector< std::pair<uint8_t, uint8_t> > write(const uint8_t& address, const std::vector< std::pair<uint8_t, uint8_t> > & data);
    std::vector<uint8_t> read(const uint8_t& address, const unsigned int length = 1);
    std::vector<uint8_t> read(const uint8_t& address, const uint8_t reg, const unsigned int length = 1);

    // Unused constructor
    iface_loopback()             = delete;

    //only this function can create the interface
    friend iface_loopback& interface_manager::getInterface< iface_loopback >(std::string const & );
  };
  

}

#endif /* CARIBOU_HAL_LOOPBACK_HPP */
