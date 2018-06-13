/**
 * Caribou Example Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_EXAMPLE_H
#define DEVICE_EXAMPLE_H

#include "constants.hpp"
#include "device.hpp"
#include "loopback.hpp"
#include "pearydevice.hpp"

namespace caribou {

/** Default device path for this device
 *
 *  Usually this is a parameter dependent only on the layout of the chip board
 *  and there should not be a necessity to change this. However, it is possible
 *  to overwrite this via the configuration using "devicepath" as key.
 */
#define DEFAULT_DEVICEPATH "/dev/null"

#define DEFAULT_ADDRESS 0x33

/** Definition of default values for the different DAC settings of the device
 *
 *  These values will be used when the configuration object does not contain
 *  the key in question, e.g.
 *  int mysetting = _config.Get("mysetting",EXAMPLE_MYSETTING);
 *  returns either the value configured or EXAMPLE_MYSETTING when not specified
 */
#define EXAMPLE_DAC_TEST 5
#define EXAMPLE_DAC_VEC 1, 3, 5, 7, 9

/** Local dictionary for register name lookup for this device
 *
 *  Add the definitions for register addresses and range here and
 *  do the name lookup like
 *  int adr = exampleDict.getAddress("threshold");
 *  in order to resolve human-readable register names to their addresses.
 */
#define EXAMPLE_REGISTERS                                                                                                   \
  {                                                                                                                         \
    {"vthreshold", register_t<>(0x5)}, {"vkrum", register_t<>(0x6, 0x0f)}, {"vadc", register_t<>(0x6)},                     \
      {"readonly", register_t<>(0x5, 0xff, true, false)}, {"writeonly", register_t<>(0x5, 0xff, false, true)},              \
      {"special", register_t<>(0x5, 0xff, true, true, true)},                                                               \
  }

#define EXAMPLE_PERIPHERY                                                                                                   \
  { {"va", PWR_OUT_1}, {"vd", PWR_OUT_2}, }

  /** Example Device class definition
   *
   *  This class implements all purely virtual functions of caribou::caribouDevice.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class ExampleDevice : public pearyDevice<iface_loopback> {

  public:
    /** Device constructor
     *
     *  The constructor must call the device::initialize() function in order to properly
     *  initialize the HAL and other components of the device. This cannot be implemented
     *  in the device class constructor since the interface type is only known once the
     *  child object (of this class) exists.
     */
    ExampleDevice(const caribou::Configuration config);
    ~ExampleDevice();

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

    void doDeviceSpecificThings(std::string arg1, int arg2);
  };

} // namespace caribou

#endif /* DEVICE_EXAMPLE_H */
