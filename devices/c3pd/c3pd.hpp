/**
 * Caribou implementation for the C3PDF chip
 */

#ifndef DEVICE_C3PD_H
#define DEVICE_C3PD_H

#include "device.hpp"
#include "pearydevice.hpp"
#include "i2c.hpp"
#include "c3pd_defaults.hpp"

namespace caribou {
  /** C3PD Device class definition
   */
  class C3PD : public pearyDevice<iface_i2c> {
    
  public:
    C3PD(const caribou::Configuration config) : pearyDevice(config) {
      this->initialize(std::string(DEFAULT_DEVICEPATH),C3PD_DEFAULT_I2C,caribou::dictionary<uint8_t>(C3PD_PERIPHERY));
    };
    ~C3PD();

    std::string getName();

    /** Initializer function for C3PD
     */
    void init();

    /** Turn on the power supply for the C3PD chip
     */
    void powerOn();

    /** Turn off the C3PD power
     */
    void powerOff();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface() {};

  private:

    // analog power supply
    // digital power supply

    // hv bias

    // I2C interface
    // reset signal pin
    // power enable pin

  };

  /** Device generator
   *
   *  This generator function is used by the device manager to create instances of
   *  this device using the abstract caribouDevice class interface. It has to be implemented
   *  for every class deriving from caribouDevice.
   */
  extern "C" {
    caribouDevice * generator(const caribou::Configuration);
  }

} //namespace caribou

#endif /* DEVICE_C3PD_H */
