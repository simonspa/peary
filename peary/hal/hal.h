#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>

namespace caribou {

  class CaribouHAL {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL();

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getDeviceID();

    /** Send I2C command(s) to the address specified containing the
     *  data supplied via the function argument
     */
    void sendCommandI2C(uint8_t address, std::vector<uint8_t> data);

    /** Send command via the SPI interface
     *
     *  The function accepts the register address to be written to as well as
     *  a vector of data words to be sent (MOSI, master out slave in).
     *  All data is send to the same address.
     *
     *  The return value is the response retrieved from the SPI interface 
     *  (MISO, master in slave out)
     *  The responses from each individual SPI command are returned in the order
     *  of the command execution.
     *  For SPI commands which do not correspond to a MISO output, the return
     *  vector is empty.
     */
    std::vector<uint8_t> sendCommandSPI(uint8_t address, std::vector<uint8_t> data);
    
  private:

  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
