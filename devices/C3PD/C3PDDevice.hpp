/**
 * Caribou implementation for the C3PD chip
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
  class C3PDDevice : public CaribouDevice<iface_i2c> {

  public:
    C3PDDevice(const caribou::Configuration config);
    ~C3PDDevice();

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

    void configureMatrix(std::string filename);

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

} // namespace caribou

#endif /* DEVICE_C3PD_H */
