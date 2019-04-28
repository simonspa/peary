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
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_CLICTD_DEFAULTS_H */
