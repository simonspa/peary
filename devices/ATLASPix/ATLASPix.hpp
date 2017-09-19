/**
 * Caribou implementation for the ATLASPixF chip
 */

#ifndef DEVICE_ATLASPix_H
#define DEVICE_ATLASPix_H

#include "ATLASPix_defaults.hpp"
#include "device.hpp"
#include "i2c.hpp"
#include "pearydevice.hpp"

namespace caribou {

  /** ATLASPix Device class definition
   */
  class ATLASPix : public pearyDevice<iface_i2c> {

  public:
    ATLASPix(const caribou::Configuration config);
    ~ATLASPix();

    std::string getName();

    /** Initializer function for ATLASPix
     */
    void configure();

    /** Turn on the power supply for the ATLASPix chip
     */
    void powerUp();

    /** Turn off the ATLASPix power
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


    void LoadConfiguration(int matrix);

    void configureClock();


  private:
    // analog power supply
    // digital power supply

    // hv bias

    // I2C interface
    // reset signal pin
    // power enable pin

    // Access to FPGA memory mapped registers
    int memfd;

    // ATLASPix control memory map address
    void* control_base;
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

#endif /* DEVICE_ATLASPix_H */
