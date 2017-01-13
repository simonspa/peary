#ifndef CARIBOU_HAL_I2C_HPP
#define CARIBOU_HAL_I2C_HPP

#include <vector>
#include <cstdint>
#include <mutex>
#include <string>

#include "log.hpp"
#include "interface_manager.hpp"

namespace caribou {

  typedef uint16_t i2c_address_t;
  typedef uint8_t  i2c_data_t;

  class iface_i2c {

  private:

    //Default constructor: private (only created by interface_manager)
    iface_i2c(std::string const & device_path) {};

  public:

    //Unused constructor
    iface_i2c()             = delete;


    /** Send command via the I2C interface
     *
     *  The function accepts the register address to be written to as well as
     *  a vector of data words to be sent. All data is send to the same address.
     */
    void sendCommand(i2c_address_t address, std::vector<i2c_data_t> data);

    
    //only this function can create the interface
    friend iface_i2c* interface_manager<iface_i2c>::getInterface(std::string const & );

  };
  
  class I2C : interface_manager<iface_i2c> {
  private:
    
    // Default constructor: private for singleton class 
    I2C() {};
    
  public:
    /* Delete unwanted functions from singleton class (C++11)
     */
    I2C(I2C const&) = delete;
    void operator=(I2C const&)    = delete;
  };


}

#endif /* CARIBOU_HAL_I2C_HPP */
