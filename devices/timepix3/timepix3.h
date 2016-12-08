/**
 * Caribou Device implementation for Timepix3
 */

#ifndef DEVICE_TIMEPIX3_H
#define DEVICE_TIMEPIX3_H

#include "device.h"
#include <string>
#include <vector>

namespace caribou {

  /** Timepix3 Device class definition
   *
   *  this class implements the required functionality to operate Timepix3 chips via the
   *  Caribou device class interface.
   */
  class timepix3 : public caribouDevice {
    
  public:
  timepix3(caribou::Configuration config) : caribouDevice(config) {};
    ~timepix3() {};

    /** Initializer function for Timepix3
     */
    void init() {};

    /** Turn on the power supply for the Timepix3 chip
     */
    void powerOn() {};

    /** Turn off the Timepix3 power
     */
    void powerOff() {};

    /** Start the data acquisition
     */
    void daqStart() {};

    /** Stop the data acquisition
     */
    void daqStop() {};

  private:

  };

  extern "C" {
    caribouDevice * generator(caribou::Configuration);
  }

} //namespace caribou

#endif /* DEVICE_TIMEPIX3_H */
