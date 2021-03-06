#ifndef DEVICE_C3PD_DEFAULTS_H
#define DEVICE_C3PD_DEFAULTS_H

#include "carboard/Carboard.hpp"
#include "utils/dictionary.hpp"

namespace caribou {

/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH BUS_I2C2

/** Default I2C address for standalone-C3PD board with unconnected I2C address lines
 */
#define C3PD_DEFAULT_I2C 0x50

/** Definition of default values for the different DAC settings for C3PD
 */
#define C3PD_VBPRE 0x38
#define C3PD_VBPCAS 0x60
#define C3PD_VBOALF 0x48
#define C3PD_VBOAHF 0x05
#define C3PD_VBLS 0x1B
#define C3PD_VBFBK 0x02
#define C3PD_VBS 0x0A
#define C3PD_VBSP 0x0A
#define C3PD_VBPREOFF 0x02
#define C3PD_VBLSOFF 0x02
#define C3PD_VBTP 0x00

#define C3PD_VDDD 1.8
#define C3PD_VDDD_CURRENT 3
#define C3PD_VDDA 1.8
#define C3PD_VDDA_CURRENT 3
#define C3PD_REF 1.0
#define C3PD_AIN 0.0

  // C3PD control
  const std::intptr_t C3PD_CONTROL_BASE_ADDRESS = 0x43C20000;
  const std::intptr_t C3PD_RESET_OFFSET = 0x88;

  const uint32_t C3PD_CONTROL_RESET_MASK = 0x1;
  const std::size_t C3PD_CONTROL_MAP_SIZE = 4096;
  const std::size_t C3PD_CONTROL_MAP_MASK = C3PD_CONTROL_MAP_SIZE - 1;

/** C3PD Routing
 *
 *  C3PD_VDDD -> SEAF F1/F2 -> (ADDR_MONITOR_U52) -> ADDR_DAC_U50 (VOUTC)
 *  C3PD_VDDA -> SEAF D5/D6 -> (ADDR_MONITOR_U56) -> ADDR_DAC_U50 (VOUTD)
 *
 *  C3PD_REF -> SEAF B1 -> ADDR_DAC_U44 (VOUTC)
 *
 *  C3PD_RSTN -> SEAF G19 -> FMC/CMOS_OUT_7
 *  C3PD_PWRE -> SEAF H18 -> FMC/CMOS_OUT_6
 *  C3PD_TPS -> SEAF H17 -> FMC/CMOS_OUT_5
 *
 *  C3PD_AOUT -> SEAF D18 -> ADDR_ADC (CH1)
 *  C3PD_AIN -> SEAF A3 -> ADDR_DAC_U44 (VOUTA)
 *
 *  C3PD_HV -> SEAF A23 -> LEMO_RA J10
 *  C3PD_PIX0 -> (unconnected on CLICpix2 board)
 *  C3PD_PIX1 -> (unconnected on CLICpix2 board)
 *  C3PD_PIX2 -> (unconnected on CLICpix2 board)
 *  C3PD_PIX3 -> (unconnected on CLICpix2 board)
 */

#define C3PD_MEMORY                                                                                                         \
  {                                                                                                                         \
    {"reset",                                                                                                               \
     memory_map(C3PD_CONTROL_BASE_ADDRESS,                                                                                  \
                C3PD_RESET_OFFSET,                                                                                          \
                C3PD_CONTROL_MAP_SIZE,                                                                                      \
                C3PD_CONTROL_MAP_MASK,                                                                                      \
                PROT_READ | PROT_WRITE)},                                                                                   \
  }

/** Dictionary for register address/name lookup for C3PD
 */
// clang-format off
#define C3PD_REGISTERS				\
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

#endif /* DEVICE_C3PD_DEFAULTS_H */
