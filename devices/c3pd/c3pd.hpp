/**
 * Caribou implementation for the C3PDF chip
 */

#ifndef DEVICE_C3PD_H
#define DEVICE_C3PD_H

#include "c3pd_defaults.hpp"
#include "device.hpp"
#include "i2c.hpp"
#include "pearydevice.hpp"

namespace caribou {

  /** C3PD Device class definition
   */
  class C3PD : public pearyDevice<iface_i2c> {

  public:
    C3PD(const caribou::Configuration config);
    ~C3PD();

    std::string getName();

    /** Initializer function for C3PD
     */
    void configure();

    /** Turn on the power supply for the C3PD chip
     */
    void powerUp();

    /** Turn off the C3PD power
     */
    void powerDown();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface(){};

    // Reset the chip
    // The reset signal is asserted for ~5us
    void reset();

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
  caribouDevice* generator(const caribou::Configuration);
  }

} // namespace caribou

#endif /* DEVICE_C3PD_H */
