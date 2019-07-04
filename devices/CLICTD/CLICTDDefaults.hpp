#ifndef DEVICE_CLICTD_DEFAULTS_H
#define DEVICE_CLICTD_DEFAULTS_H

#include "carboard/Carboard.hpp"
#include "utils/dictionary.hpp"

namespace caribou {

/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH BUS_I2C2

/** Default I2C address for CLICTD chip-board with unconnected I2C address lines
 */
#define CLICTD_DEFAULT_I2C 0x50

#define CLICTD_VDDD 1.8
#define CLICTD_VDDD_CURRENT 3
#define CLICTD_VDDA 1.8
#define CLICTD_VDDA_CURRENT 3
#define CLICTD_PWELL 3.054
#define CLICTD_PWELL_CURRENT 3
#define CLICTD_SUB 3.057
#define CLICTD_SUB_CURRENT 3

  // CLIcpix2 control
  const std::intptr_t CLICPIX2_CONTROL_BASE_ADDRESS = 0x43C20000;
  const std::intptr_t CLICPIX2_RESET_OFFSET = 0;
  const uint32_t CLICPIX2_CONTROL_RESET_MASK = 0x1;
  const std::size_t CLICPIX2_CONTROL_MAP_SIZE = 4096;
  const std::size_t CLICPIX2_CONTROL_MAP_MASK = CLICPIX2_CONTROL_MAP_SIZE - 1;

#define CLICPIX2_MEMORY                                                                                                     \
  {                                                                                                                         \
    {                                                                                                                       \
      "reset", memory_map(CLICPIX2_CONTROL_BASE_ADDRESS,                                                                    \
                          CLICPIX2_RESET_OFFSET,                                                                            \
                          CLICPIX2_CONTROL_MAP_SIZE,                                                                        \
                          CLICPIX2_CONTROL_MAP_MASK,                                                                        \
                          PROT_READ | PROT_WRITE)                                                                           \
    }                                                                                                                       \
  }

/** Dictionary for register address/name lookup for CLICTD
 */
// clang-format off
#define CLICTD_REGISTERS				\
  {						\
    {"globalconfig", register_t<>(0x00, 0x07)},		\
    {"tpen", register_t<>(0x00, 0x01)},		\
    {"tpexten", register_t<>(0x00, 0x02)},		\
    {"pwrexten", register_t<>(0x00, 0x04)},		\
    {"internalstrobes", register_t<>(0x01, 0x03)},		\
    {"tpint", register_t<>(0x01, 0x01)},		\
    {"pwrint", register_t<>(0x01, 0x02)},		\
    {"externaldacsel", register_t<>(0x02,0x1F)},		\
    {"oir", register_t<>(0x02, 0x80)},		\
    {"monitordacsel", register_t<>(0x03, 0x1F)},		\
    {"matrixconfig", register_t<>(0x04, 0x1F)},		\
    {"photoncnt", register_t<>(0x04, 0x10)},		\
    {"nocompress", register_t<>(0x04, 0x08)},		\
    {"longcnt", register_t<>(0x04, 0x04)},		\
    {"totdiv", register_t<>(0x04, 0x03)},		\
    {"configctrl", register_t<>(0x05, 0x13, false)},		\
    {"configdata_lsb", register_t<>(0x06)},		\
    {"configdata_msb", register_t<>(0x07)},		\
    {"configdata", register_t<>(0x06, 0xFF, false, true, true)},		\
    {"readoutctrl", register_t<>(0x08, 0x03)},		\
    {"roint", register_t<>(0x08, 0x01)},		\
    {"roexten", register_t<>(0x08, 0x02)},		\
    {"tpulsectrl_lsb", register_t<>(0x0A)},		\
    {"tpulsectrl_msb", register_t<>(0x0B)},		\
    {"tpulsectrl", register_t<>(0x0A, 0xFF, false, true, true)},		\
    {"vbiasresettransistor", register_t<>(0x10)},		\
    {"vreset", register_t<>(0x11)},		\
    {"vbiaslevelshift", register_t<>(0x12, 0x3F)},		\
    {"vanalog1_lsb", register_t<>(0x13)},		\
    {"vanalog1_msb", register_t<>(0x14, 0x01)},		\
    {"vanalog1", register_t<>(0x13, 0xFF, true, true, true)},		\
    {"vanalog2", register_t<>(0x15)},		\
    {"vbiaspreampn", register_t<>(0x16, 0x3F)},		\
    {"vncasc", register_t<>(0x17)},		\
    {"vpcasc", register_t<>(0x18)},		\
    {"vfbk", register_t<>(0x19)},		\
    {"vbiasikrum", register_t<>(0x1A, 0x3F)},		\
    {"vbiasdiscn", register_t<>(0x1B, 0x3F)},		\
    {"vbiasdiscp", register_t<>(0x1C, 0x3F)},		\
    {"vbiasdac", register_t<>(0x1D, 0x3F)},		\
    {"vthreshold_lsb", register_t<>(0x1E)},		\
    {"vthreshold_msb", register_t<>(0x1F, 0x01)},		\
    {"vthreshold", register_t<>(0x1E, 0xFF, true, true, true)},		\
    {"vncasccomp", register_t<>(0x20)},		\
    {"vbiaslevelshiftstby", register_t<>(0x21, 0x3F)},		\
    {"vbiaspreampnstby", register_t<>(0x22, 0x3F)},		\
    {"vbiasdiscnstby", register_t<>(0x23, 0x3F)},		\
    {"vbiasdiscpstby", register_t<>(0x24, 0x3F)},		\
    {"vbiasdacstby", register_t<>(0x25, 0x3F)},		\
    {"vbiasslowbuffer", register_t<>(0x26)},		\
    {"adjustdacrange", register_t<>(0x27)},		\
    {"vlvdsd", register_t<>(0x28, 0x0F)},		\
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_CLICTD_DEFAULTS_H */
