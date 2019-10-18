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

#define CLICTD_MAX_CONF_RETRY 15

  // CLICTD readout
  const std::intptr_t CLICTD_READOUT_BASE_ADDRESS = 0x43C70000;
  const std::intptr_t CLICTD_READOUT_LSB = 2;
  const std::intptr_t CLICTD_READOUT_RDFIFO_OFFSET = 0;
  const std::intptr_t CLICTD_READOUT_RDSTATUS_OFFSET = 4;
  const std::intptr_t CLICTD_READOUT_RDCONTROL_OFFSET = 8;
  const std::intptr_t CLICTD_READOUT_CHIPCONTROL_OFFSET = 12;
  const std::intptr_t CLICTD_READOUT_SHUTTERTIMEOUT_OFFSET = 16;
  const std::size_t CLICTD_READOUT_MAP_SIZE = 4096;
  const std::size_t CLICTD_READOUT_MAP_MASK = CLICTD_READOUT_MAP_SIZE - 1;

// CLICTD output signals
#define CLICTD_READOUT_START 0x1
#define CLICTD_POWER_ENABLE 0x2
#define CLICTD_TESTPULSE 0x4
#define CLICTD_SHUTTER 0x8
#define CLICTD_RESET 0x16

// clang-format off

#define CLICTD_MEMORY                                                         \
  {                                                                           \
    {"rdfifo",                                                                \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   CLICTD_READOUT_RDFIFO_OFFSET,                              \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ)},                                               \
    {"rdstatus",                                                              \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   CLICTD_READOUT_RDSTATUS_OFFSET,                            \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ)},                                               \
    {"rdcontrol",                                                             \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   CLICTD_READOUT_RDCONTROL_OFFSET,                           \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"chipcontrol",                                                           \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   CLICTD_READOUT_CHIPCONTROL_OFFSET,                         \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"shuttertimeout",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   CLICTD_READOUT_SHUTTERTIMEOUT_OFFSET,                      \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"wgcontrol",                                                             \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (5 << CLICTD_READOUT_LSB),                                 \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"wgstatus",                                                              \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (6 << CLICTD_READOUT_LSB),                                 \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ)},                                               \
    {"wgcapacity",                                                            \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (7 << CLICTD_READOUT_LSB),                                 \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ)},                                               \
    {"wgconfruns",                                                            \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (8 << CLICTD_READOUT_LSB),                                 \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"wgpatterntime",                                                         \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (9 << CLICTD_READOUT_LSB),                                 \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"wgpatternoutput",                                                       \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (10 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"wgpatterntriggers",                                                     \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (11 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsfifodata_lsb",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (12 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsfifodata_msb",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (13 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsstatus",                                                              \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (14 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tscontrol",                                                             \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (15 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsedgeconf",                                                            \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (16 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsinittime_lsb",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (17 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"tsinittime_msb",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (18 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"chipsignal_enable",                                                     \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (19 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"pulser_periods",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (20 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"pulser_time_high",                                                      \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (21 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"pulser_time_low",                                                       \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (22 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)},                                  \
    {"pulser_control",                                                        \
        memory_map(CLICTD_READOUT_BASE_ADDRESS,                               \
                   (23 << CLICTD_READOUT_LSB),                                \
                   CLICTD_READOUT_MAP_SIZE,                                   \
                   CLICTD_READOUT_MAP_MASK,                                   \
                   PROT_READ | PROT_WRITE)}                                   \
  }

/** Dictionary for register address/name lookup for CLICTD
 */

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
    {"nocompress", register_t<>(0x04, 0x08, true, true, true)},		\
    {"longcnt", register_t<>(0x04, 0x04, true, true, true)},		\
    {"totdiv", register_t<>(0x04, 0x03)},		\
    {"configctrl", register_t<>(0x05, 0x13, false)},		\
    {"configdata_lsb", register_t<>(0x06)},		\
    {"configdata_msb", register_t<>(0x07)},		\
    {"configdata", register_t<>(0x06, 0xFF, true, true, true)},		\
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
