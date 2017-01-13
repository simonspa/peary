#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>
#include <cstdint>

namespace caribou {

  class caribouHAL {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL(uint8_t interface);

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getDeviceID();

    /** Send command to the device interface
     */
    std::vector<uint8_t> sendCommand(uint8_t address, std::vector<uint8_t> data);

  private:

    /** Interface of the configured device
     */
    uint8_t _iface;
    
  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
