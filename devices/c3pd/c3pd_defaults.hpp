#ifndef DEVICE_C3PD_DEFAULTS_H
#define DEVICE_C3PD_DEFAULTS_H

#include "constants.hpp"
#include "dictionary.hpp"

namespace caribou {

  /** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
   */
#define DEFAULT_DEVICEPATH BUS_I2C2


  /** Definition of default values for the different DAC settings for C3PD
   */
#define C3PD_VBPRE    0x38
#define C3PD_VBPCAS   0x60
#define C3PD_VBOALF   0x48
#define C3PD_VBOAHF   0x05
#define C3PD_VBLS     0x1B
#define C3PD_VBFBK    0x02
#define C3PD_VBS      0x0A
#define C3PD_VBSP     0x0A
#define C3PD_VBPREOFF 0x02
#define C3PD_VBLSOFF  0x02
#define C3PD_VBTP     0x00



  /** Dictionary for register address/name lookup for C3PD
   */
  class c3pdDict : public dictionary<c3pdDict> {};
  template<>
  std::map<std::string,registerConfig<> > dictionary<c3pdDict>::_registers = {
    {"gcr",      registerConfig<>(0x00,255)},
    {"isg",      registerConfig<>(0x01,255)},
    {"ani",      registerConfig<>(0x02,255)},
    {"ano",      registerConfig<>(0x03,255)},
    {"vbpre",    registerConfig<>(0x04,255)},
    {"vbpcas",   registerConfig<>(0x05,255)},
    {"vboalf",   registerConfig<>(0x06,255)},
    {"vboahf",   registerConfig<>(0x07,255)},
    {"vbls",     registerConfig<>(0x08,255)},
    {"vbfbk",    registerConfig<>(0x09,255)},
    {"vbs",      registerConfig<>(0x0A,255)},
    {"vbsp",     registerConfig<>(0x0B,255)},
    {"vbpreoff", registerConfig<>(0x0C,255)},
    {"vblsoff",  registerConfig<>(0x0D,255)},
    {"vbtp",     registerConfig<>(0x0E,255)}
  };

} //namespace caribou

#endif /* DEVICE_C3PD_DEFAULTS_H */
