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

  private:
    
  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
