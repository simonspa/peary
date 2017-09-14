/**
 * Caribou implementation for the CCPDv5 chip
 */

#ifndef DEVICE_CCPDV5_H
#define DEVICE_CCPDV5_H

#include "device.hpp"
#include "i2c.hpp"
#include "pearydevice.hpp"

namespace caribou {

  /** CCPDV5 Device class definition
   */
  class CCPDV5 : public pearyDevice<iface_i2c> {

  public:
    CCPDV5(const caribou::Configuration config);
    ~CCPDV5();

    std::string getName();

    /** Initializer function for CCPDV5
     */
    void configure();

    /** Turn on the power supply for the CCPDV5 chip
     */
    void powerUp();

    /** Turn off the CCPDV5 power
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

#endif /* DEVICE_CCPDV5_H */
