#ifndef DEVICE_TIMEPIX3_DEFAULTS_H
#define DEVICE_TIMEPIX3_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

namespace caribou {

/** Default device path for this device: SPI interface
 */
#define DEFAULT_DEVICEPATH BUS_I2C2
#define DEFAULT_DEVICEADDR 0x0

// Default register values for Timepix3 chips

#define TPX3_VPREAMP_NCAS 128
#define TPX3_IBIAS_IKRUM 10
#define TPX3_VFBK 128
#define TPX3_VTHRESH_FINE 168
#define TPX3_VTHRESH_COARSE 6
#define TPX3_IBIAS_DISCS1_ON 128
#define TPX3_IBIAS_DISCS1_OFF 8
#define TPX3_IBIAS_DISCS2_ON 128
#define TPX3_IBIAS_DISCS2_OFF 8
#define TPX3_IBIAS_PIXELDAC 128
#define TPX3_IBIAS_TPBUFIN 128
#define TPX3_IBIAS_TPBUFOUT 128
#define TPX3_VTP_COARSE 128
#define TPX3_VTP_FINE 256
#define TPX3_IBIAS_CP_PLL 128
#define TPX3_PLL_VCNTRL 128

#define TIMEPIX3_PERIPHERY                                                                                                  \
  {}

} // namespace caribou

#endif /* DEVICE_TIMEPIX3_DEFAULTS_H */
