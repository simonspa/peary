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

  
  
  typedef uint16_t i2c_address_t;
  typedef uint8_t  i2c_data_t;

  class iface_i2c {

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

    //Path of the device
    const std::string devicePath;

    //Protects access to the bus
    std::mutex mutex;

  public:

    //Unused constructor
    iface_i2c()             = delete;


    /** Write access via the I2C interface
     *
     *  The function accepts the register address to be written to as well as
     *  a vector of data words to be sent. All data is send to the same address.
     *
     * It can throw CommunicationError
     */
    void write(i2c_address_t const address, std::vector<i2c_data_t> & data);

    /** Read access via the I2C interface
     *
     *  The function accepts the register address to be read to, a destination vector of
     *  data words and number of read bytes. All data is read from the same address.
     *
     * It can throw CommunicationError.
     */
    void read(i2c_address_t const address,  std::vector<i2c_data_t> & data, const unsigned int length);

    
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
