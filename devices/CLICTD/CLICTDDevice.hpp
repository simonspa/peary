/**
 * Caribou implementation for the CLICTD chip
 */

#ifndef DEVICE_CLICTD_H
#define DEVICE_CLICTD_H

#include "device/CaribouDevice.hpp"
#include "interfaces/I2C/i2c.hpp"

#include "CLICTDDefaults.hpp"

namespace caribou {

  /** CLICTD Device class definition
   */
  class CLICTDDevice : public CaribouDevice<iface_i2c> {

  public:
    CLICTDDevice(const caribou::Configuration config);
    ~CLICTDDevice();

    /** Initializer function for CLICTD
     */
    void configure();

    /** Turn on the power supply for the CLICTD chip
     */
    void powerUp();

    /** Turn off the CLICTD power
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

    // Reset the chip
    void reset();

    void setSpecialRegister(std::string name, uint32_t value);
    uint32_t getSpecialRegister(std::string name);
  };

} // namespace caribou

#endif /* DEVICE_CLICTD_H */
