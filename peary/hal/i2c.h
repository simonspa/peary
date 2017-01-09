#ifndef CARIBOU_HAL_I2C_H
#define CARIBOU_HAL_I2C_H

#include <vector>
#include <cstdint>

namespace caribou {

  class i2c {

  public:
    /** Default constructor for creating a new I2C communicator
     */
    i2c() {};

    /** Default destructor for I2C objects
     */
    ~i2c() {};

    /** Send I2C command(s) to the address specified containing the
     *  data supplied via the function argument
     */
    void sendCommand(uint8_t address, std::vector<uint8_t> data);

  private:

  }; //class I2C

} //namespace caribou

#endif /* CARIBOU_HAL_I2C_H */
