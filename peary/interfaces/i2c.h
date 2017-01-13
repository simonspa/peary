#ifndef CARIBOU_HAL_I2C_H
#define CARIBOU_HAL_I2C_H

#include <vector>
#include <cstdint>
#include <mutex>

namespace caribou {

    /* I2C command interface class
   */
  class iface_i2c {

    /** Default constructor: private for singleton class
     */
    iface_i2c() {};

  public:
    /** Get instance of the singleton I2C interface class
     *  The below function is thread-safe in C++11 and can thus
     *  be called from several HAL instances concurrently.
     */
    static iface_i2c * getInterface() {
      static iface_i2c instance;
      return &instance;
    }

    /* Delete unwanted functions from singleton class (C++11)
     */
    iface_i2c(iface_i2c const&)             = delete;
    void operator=(iface_i2c const&)  = delete;

    /** Send command via the I2C interface
     *
     *  The function accepts the register address to be written to as well as
     *  a vector of data words to be sent. All data is send to the same address.
     */
    void sendCommand(uint8_t address, std::vector<uint8_t> data);

  private:

    /** Sending a single I2C command and reading the return value
     */
    void sendCommand(uint8_t address, uint8_t data);

    /** Private mutex to lock driver access
     */
    std::mutex mutex;
    
  }; //class iface_i2c

} //namespace caribou

#endif /* CARIBOU_HAL_I2C_H */
