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

  
  
  typedef uint8_t i2c_t;

  class iface_i2c : public Interface<i2c_t, i2c_t>  {

  private:

    //Default constructor: private (only created by interface_manager)
    //
    //It can throw DeviceException
    iface_i2c(std::string const & device_path);

    //Set I2C address before read/write access
    //
    //It can throw CommunicationError
    inline void setAddress(i2c_address_t const address);
    
    //Descriptor of the device
    int i2cDesc;

    //Protects access to the bus
    std::mutex mutex;

  public:

    //Unused constructor
    iface_i2c()             = delete;

    //only this function can create the interface
    friend iface_i2c* interface_manager::getInterface< iface_i2c >(std::string const & );

  };
  

}

#endif /* CARIBOU_HAL_I2C_HPP */
