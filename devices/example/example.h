/**
 * Caribou Example Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_EXAMPLE_H
#define DEVICE_EXAMPLE_H

#include "device.h"

namespace caribou {

  /** Example Device class definition
   *
   *  this class implements all purely virtual functions of caribou::caribouDevice.
   *  Applications can then control this device via the Caribou device class interface.
   */
  class example : public caribouDevice {
    
  public:
  example() : caribouDevice(caribou::Configuration()) {};
    ~example();
    
    /** Initializer function for Example
     */
    void init();

    /** Turn on the power supply for the Example chip
     */
    void powerOn();

    /** Turn off the Example power
     */
    void powerOff();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

  private:

  };

  /** Device generator
   *
   *  This generator function is used by the device manager to create instances of
   *  this device using the abstract caribouDevice class interface. It has to be implemented
   *  for every class deriving from caribouDevice.
   */
  extern "C" {
    caribouDevice * generator();
  }

} //namespace caribou

#endif /* DEVICE_EXAMPLE_H */
