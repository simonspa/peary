/**
 * Caribou Example Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_EXAMPLE_H
#define DEVICE_EXAMPLE_H

#include "device.hpp"
#include "constants.hpp"
#include "example_defaults.hpp"

namespace caribou {

  /** Example Device class definition
   *
   *  This class implements all purely virtual functions of caribou::caribouDevice.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class example : public caribouDevice {
    
  public:
    /** Device constructor
     *
     *  The constructor must call the device::initialize() function in order to properly
     *  initialize the HAL and other components of the device. This cannot be implemented
     *  in the device class constructor since the interface type is only known once the
     *  child object (of this class) exists.
     */
    example(const caribou::Configuration config) : caribouDevice(config), exampleDict(C3PD_DICT) {
      // Initialize the device and HAL object:
      this->initialize(std::string(DEFAULT_DEVICEPATH), caribou::dictionary<uint8_t>(C3PD_PERIPHERY));
    };
    ~example();
    
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

    /** Example function only available in this derived device class
     */
    void exampleCall();

  private:

    /** Device interface
     *
     *  Specifies the device hardware interface to be instanciated by the HAL.
     *  Returns a value of type caribou::IFACE.
     */
    IFACE interface() { return IFACE::LOOPBACK; }

    dictionary<> exampleDict;
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

#endif /* DEVICE_EXAMPLE_H */
