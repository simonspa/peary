#ifndef CARIBOU_HAL_I2C_HPP
#define CARIBOU_HAL_I2C_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <mutex>

#include "exceptions.hpp"
#include "interface_manager.hpp"
#include "interface.hpp"

namespace caribou {

  typedef uint8_t i2c_address_t;
  typedef uint8_t i2c_t;
  typedef uint8_t i2c_reg_t;


  class iface_i2c : public Interface<i2c_t, i2c_reg_t, i2c_t> {

  private:

    //Default constructor: private (only created by interface_manager)
    //
    //It can throw DeviceException
    iface_i2c(std::string const & device_path);

    virtual ~iface_i2c();
    
    //Set I2C address before read/write access
    //
    //It can throw CommunicationError
    inline void setAddress(i2c_address_t const address);
    
    //Descriptor of the device
    int i2cDesc;

    //Protects access to the bus
    std::mutex mutex;

    template<typename T>
    friend class caribouHAL;

  private:
    i2c_t write(const i2c_address_t& address, const i2c_t& data );
    std::vector<i2c_t> write(const i2c_address_t& address, const std::vector<i2c_t>& data );
    std::pair<i2c_reg_t, i2c_t> write(const i2c_address_t& address, const std::pair<i2c_reg_t, i2c_t> & data);
    std::vector<i2c_t> write(const i2c_address_t& address, const i2c_reg_t & reg, const std::vector< i2c_t > & data);

    //length must be 1
    std::vector<i2c_t> read(const i2c_address_t& address, const unsigned int length = 1);
    //length must be 32
    std::vector<i2c_t> read(const i2c_address_t& address, const i2c_reg_t reg, const unsigned int length = 32);

    // Special functions to read/write to devices with up to 16bit register
    std::vector<i2c_t> wordwrite(const i2c_address_t& address, const uint16_t & reg, const std::vector< i2c_t > & data);
    std::vector<i2c_t> wordread(const i2c_address_t& address, const uint16_t reg, const unsigned int length = 1);
    //Unused constructor
    iface_i2c()             = delete;

    //only this function can create the interface
    friend iface_i2c& interface_manager::getInterface< iface_i2c >(std::string const & );
  };
  

}

#endif /* CARIBOU_HAL_I2C_HPP */
