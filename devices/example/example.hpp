/**
 * Caribou Example Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_EXAMPLE_H
#define DEVICE_EXAMPLE_H

#include "constants.hpp"
#include "device.hpp"
#include "example_defaults.hpp"
#include "loopback.hpp"
#include "pearydevice.hpp"

namespace caribou {

  /** Example Device class definition
   *
   *  This class implements all purely virtual functions of caribou::caribouDevice.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class example : public pearyDevice<iface_loopback> {

  public:
    /** Device constructor
     *
     *  The constructor must call the device::initialize() function in order to properly
     *  initialize the HAL and other components of the device. This cannot be implemented
     *  in the device class constructor since the interface type is only known once the
     *  child object (of this class) exists.
     */
    example(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), DEFAULT_ADDRESS) {
      _periphery.add("vd", PWR_OUT_1);
      _registers.add(EXAMPLE_REGISTERS);
    };
    ~example();

    /** Return human-readable name of this device
     */
    std::string getName();

    /** Initializer function for Example chip
     */
    void init(){};

    /** Turn on the power supply for the Example chip
     */
    void powerUp();

    /** Turn off the Example power
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
    void powerStatusLog(){};

    void exploreInterface(){};

    /** Example function only available in this derived device class
     */
    void exampleCall();

    void setSpecialRegister(std::string name, uint32_t value);
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

#endif /* DEVICE_EXAMPLE_H */
