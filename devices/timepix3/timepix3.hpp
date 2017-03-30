/**
 * Caribou Device implementation for Timepix3
 */

#ifndef DEVICE_TIMEPIX3_H
#define DEVICE_TIMEPIX3_H

#include "device.hpp"
#include "pearydevice.hpp"
#include "i2c.hpp"
#include "timepix3_defaults.hpp"

#include <string>
#include <vector>

namespace caribou {

  /** Timepix3 Device class definition
   *
   *  this class implements the required functionality to operate Timepix3 chips via the
   *  Caribou device class interface.
   */
  class timepix3 : public pearyDevice<iface_i2c> {
    
  public:
    timepix3(const caribou::Configuration config) :
      pearyDevice(config, std::string(DEFAULT_DEVICEPATH), DEFAULT_DEVICEADDR) {};
    ~timepix3() {};

    std::string getName();
    
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

    /** Report power status
     */
    void powerStatusLog() {};

    void exploreInterface() {};
  private:

  };

  extern "C" {
    caribouDevice * generator(const caribou::Configuration);
  }

} //namespace caribou

#endif /* DEVICE_TIMEPIX3_H */
