#ifndef DEVICE_ATLASPix_DEFAULTS_H
#define DEVICE_ATLASPix_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

namespace caribou {

/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH BUS_I2C2

/** Default I2C address for standalone-ATLASPix board with unconnected I2C address lines
 */
#define ATLASPix_DEFAULT_I2C 0x50

/** Definition of default values for the different DAC settings for ATLASPix
 */
#define ATLASPix_VBPRE 0x38
#define ATLASPix_VBPCAS 0x60
#define ATLASPix_VBOALF 0x48
#define ATLASPix_VBOAHF 0x05
#define ATLASPix_VBLS 0x1B
#define ATLASPix_VBFBK 0x02
#define ATLASPix_VBS 0x0A
#define ATLASPix_VBSP 0x0A
#define ATLASPix_VBPREOFF 0x02
#define ATLASPix_VBLSOFF 0x02
#define ATLASPix_VBTP 0x00

#define ATLASPix_VDDD 1.8
#define ATLASPix_VDDD_CURRENT 1
#define ATLASPix_VDDA 1.8
#define ATLASPix_VDDA_CURRENT 1
#define ATLASPix_VSSA 1.0
#define ATLASPix_VSSA_CURRENT 0.5

#define ATLASPix_GndDACPix_M2 1.8
#define ATLASPix_VMinusPix_M2 0.0
#define ATLASPix_GatePix_M2   2.2


  // ATLASPix control
  const std::intptr_t ATLASPix_CONTROL_BASE_ADDRESS = 0x43C20000;
  const std::intptr_t ATLASPix_RESET_OFFSET = 0x88;

  const uint32_t ATLASPix_CONTROL_RESET_MASK = 0x1;
  const std::size_t ATLASPix_CONTROL_MAP_SIZE = 4096;
  const std::size_t ATLASPix_CONTROL_MAP_MASK = ATLASPix_CONTROL_MAP_SIZE - 1;


/** Dictionary for register address/name lookup for ATLASPix
 */
// clang-format off
#define ATLASPix_REGISTERS				\
  {						\
    {"gcr", register_t<>(0x00)},		\
    {"dlc", register_t<>(0x00, 0x60)},		\
    {"pwrexen", register_t<>(0x00, 0x10)},	\
    {"tpexten", register_t<>(0x00, 0x02)},	\
    {"tpen", register_t<>(0x00, 0x01)},		\
    {"isg", register_t<>(0x01)},		\
    {"pwri", register_t<>(0x01, 0x02)},		\
    {"tpis", register_t<>(0x01, 0x01)},		\
    {"ani", register_t<>(0x02)},		\
    {"oir", register_t<>(0x02, 0x80)},		\
    {"ai", register_t<>(0x02, 0x0F)},		\
    {"ano", register_t<>(0x03)},		\
    {"pmc", register_t<>(0x03, 0x30)},		\
    {"ao", register_t<>(0x03, 0x0F)},		\
    {"vbpre", register_t<>(0x04)},		\
    {"vbpcas", register_t<>(0x05)},		\
    {"vboalf", register_t<>(0x06)},		\
    {"vboahf", register_t<>(0x07)},		\
    {"vbls", register_t<>(0x08)},		\
    {"vbfbk", register_t<>(0x09)},		\
    {"vbs", register_t<>(0x0A)},		\
    {"vbsp", register_t<>(0x0B)},		\
    {"vbpreoff", register_t<>(0x0C)},		\
    {"vblsoff", register_t<>(0x0D)},		\
    {"vbtp", register_t<>(0x0E)},		\
    {"tpcea", register_t<>(0x10)},		\
    {"tpceb", register_t<>(0x11)},		\
    {"tpcec", register_t<>(0x12)},		\
    {"tpced", register_t<>(0x13)},		\
    {"tpcee", register_t<>(0x14)},		\
    {"tpcef", register_t<>(0x15)},		\
    {"tpceg", register_t<>(0x16)},		\
    {"tpceh", register_t<>(0x17)},		\
    {"tpcei", register_t<>(0x18)},		\
    {"tpcej", register_t<>(0x19)},		\
    {"tpcek", register_t<>(0x1A)},		\
    {"tpcel", register_t<>(0x1B)},		\
    {"tpcem", register_t<>(0x1C)},		\
    {"tpcen", register_t<>(0x1D)},              \
    {"tpceo", register_t<>(0x1E)},		\
    {"tpcep", register_t<>(0x1F)},		\
    {"tprea", register_t<>(0x20)},		\
    {"tpreb", register_t<>(0x21)},		\
    {"tprec", register_t<>(0x22)},		\
    {"tpred", register_t<>(0x23)},		\
    {"tpree", register_t<>(0x24)},		\
    {"tpref", register_t<>(0x25)},		\
    {"tpreg", register_t<>(0x26)},		\
    {"tpreh", register_t<>(0x27)},		\
    {"tprei", register_t<>(0x28)},		\
    {"tprej", register_t<>(0x29)},		\
    {"tprek", register_t<>(0x2A)},		\
    {"tprel", register_t<>(0x2B)},		\
    {"tprem", register_t<>(0x2C)},		\
    {"tpren", register_t<>(0x2D)},		\
    {"tpreo", register_t<>(0x2E)},		\
    {"tprep", register_t<>(0x2F)},		\
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_ATLASPix_DEFAULTS_H */
