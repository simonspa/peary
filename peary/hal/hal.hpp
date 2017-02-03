#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>
#include <cstdint>

namespace caribou {

  class caribouHAL {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL(IFACE interface, std::string device_path);

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getDeviceID();

    /** Send command to the device interface
     */
    std::vector<uint8_t>& write(std::vector<uint8_t> address, std::vector<uint8_t> data);
    std::vector<uint8_t>& write(std::vector<uint8_t> address, std::vector<std::pair<uint8_t,uint8_t>> data);

    // Throw exception for SPI, read command for I2C
    std::vector<uint8_t> read(uint8_t address, uint8_t length);
    
  private:

    /** Interface of the configured device
     */
    uint8_t _iface;
    
  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
