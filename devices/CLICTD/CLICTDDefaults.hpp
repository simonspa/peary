#ifndef DEVICE_CLICTD_DEFAULTS_H
#define DEVICE_CLICTD_DEFAULTS_H

#include "carboard/carboard.hpp"
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
    {"matrixconfig", register_t<>(0x04)},		\
    {"configctrl", register_t<>(0x05)},		\
    {"configstage", register_t<>(0x05, 0x03)},		\
    {"configsend", register_t<>(0x05, 0x10)},		\
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
    {"vbiaslevelshift", register_t<>(0x12)},		\
    {"vanalog1_lsb", register_t<>(0x13)},		\
    {"vanalog1_msb", register_t<>(0x14, 0x01)},		\
    {"vanalog1", register_t<>(0x13, 0xFF, true, true, true)},		\
    {"vanalog2", register_t<>(0x15)},		\
    {"vbiaspreampn", register_t<>(0x16)},		\
    {"vncasc", register_t<>(0x17)},		\
    {"vpcasc", register_t<>(0x18)},		\
    {"vfbk", register_t<>(0x19)},		\
    {"vbiasikrum", register_t<>(0x1A)},		\
    {"vbiasdiscn", register_t<>(0x1B)},		\
    {"vbiasdiscp", register_t<>(0x1C)},		\
    {"vbiasdac", register_t<>(0x1D)},		\
    {"vthreshold_lsb", register_t<>(0x1E)},		\
    {"vthreshold_msb", register_t<>(0x1F, 0x01)},		\
    {"vthreshold", register_t<>(0x1E, 0xFF, true, true, true)},		\
    {"vncasccomp", register_t<>(0x20)},		\
    {"vbiaslevelshiftstby", register_t<>(0x21)},		\
    {"vbiaspreampnstby", register_t<>(0x22)},		\
    {"vbiasdiscnstby", register_t<>(0x23)},		\
    {"vbiasdiscpstby", register_t<>(0x24)},		\
    {"vbiasdacstby", register_t<>(0x25)},		\
    {"vbiasslowbuffer", register_t<>(0x26)},		\
    {"adjustdacrange", register_t<>(0x27)},		\
    {"vlvdsd", register_t<>(0x28, 0x0F)},		\
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_CLICTD_DEFAULTS_H */
