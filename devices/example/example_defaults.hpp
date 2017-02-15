#ifndef DEVICE_EXAMPLE_DEFAULTS_H
#define DEVICE_EXAMPLE_DEFAULTS_H

#include "dictionary.hpp"
#include "carboard.hpp"

namespace caribou {

  /** Default device path for this device
   *
   *  Usually this is a parameter dependent only on the layout of the chip board
   *  and there should not be a necessity to change this. However, it is possible
   *  to overwrite this via the configuration using "devicepath" as key.
   */
#define DEFAULT_DEVICEPATH "/dev/null"

  /** Definition of default values for the different DAC settings of the device
   *
   *  These values will be used when the configuration object does not contain
   *  the key in question, e.g.
   *  int mysetting = _config.Get("mysetting",EXAMPLE_MYSETTING);
   *  returns either the value configured or EXAMPLE_MYSETTING when not specified
   */
#define EXAMPLE_DAC_TEST 5
#define EXAMPLE_DAC_VEC  1,3,5,7,9

  /** Local dictionary for register name lookup for this device
   *
   *  Add the definitions for register addresses and range here and 
   *  do the name lookup like
   *  int adr = exampleDict.getAddress("threshold");
   *  in order to resolve human-readable register names to their addresses.
   */
#define C3PD_DICT \
  {						\
    {"vthreshold", registerConfig<>(5,255)},	\
    {"vkrum",      registerConfig<>(6,255)},	\
    {"vadc",       registerConfig<>(6,255)}	\
  }


#define C3PD_PERIPHERY \
  {									\
    {"va", registerConfig<>(ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTC,16)},	\
    {"vd", registerConfig<>(ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTD,16)},	\
  }

} //namespace caribou

#endif /* DEVICE_EXAMPLE_DEFAULTS_H */
