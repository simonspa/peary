#ifndef DEVICE_EXAMPLE_DEFAULTS_H
#define DEVICE_EXAMPLE_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

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

} // namespace caribou

#endif /* DEVICE_EXAMPLE_DEFAULTS_H */
