#ifndef DEVICE_ATLASPix_DEFAULTS_H
#define DEVICE_ATLASPix_DEFAULTS_H

#include "carboard/Carboard.hpp"
#include "clockgenerator/clk_6-100_8-100_ref-0FMC.h"
#include "utils/dictionary.hpp"

namespace caribou {

// clang-format off
/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH BUS_I2C2

/** Default I2C address for standalone-ATLASPix board with unconnected I2C address lines
 */
#define ATLASPix_DEFAULT_I2C 0b0011000

  /** Definition of default values for the different DAC settings for ATLASPix
   */
#define ATLASPix_VCC25 2.5
#define ATLASPix_VCC25_CURRENT 2

#define ATLASPix_VDDD 1.85
#define ATLASPix_VDDD_CURRENT 2
#define ATLASPix_VDDRam 1.85
#define ATLASPix_VDDRam_CURRENT 2
#define ATLASPix_VDDHigh 1.85
#define ATLASPix_VDDHigh_CURRENT 2
#define ATLASPix_VDDA 1.85
#define ATLASPix_VDDA_CURRENT 2
#define ATLASPix_VSSA 1.2
#define ATLASPix_VSSA_CURRENT 2


#define ATLASPix_GndDACPix_M2 0
#define ATLASPix_VMinusPix_M2 0.65
#define ATLASPix_GatePix_M2 2.2

#define ATLASPix_GndDACPix_M1 0
#define ATLASPix_VMinusPix_M1 0.65
#define ATLASPix_GatePix_M1 2.2

#define ATLASPix_GndDACPix_M1ISO 0
#define ATLASPix_VMinusPix_M1ISO 0.65
#define ATLASPix_GatePix_M1ISO 2.2

#define ATLASPix_BLPix_M1 0.8
#define ATLASPix_BLPix_M1ISO 0.8
#define ATLASPix_BLPix_M2 0.8

#define ATLASPix_ThPix_M1 0.95
#define ATLASPix_ThPix_M1ISO 0.95
#define ATLASPix_ThPix_M2 0.95

#define ncol_m1 25
#define nrow_m1 400

#define ncol_m1iso 25
#define nrow_m1iso 400

#define ncol_m2 56
#define nrow_m2 320

#define ATLASPix_mask_X 1
#define ATLASPix_mask_Y 400

#define TuningMaxCount 1000
#define Tuning_timeout 100

  // ATLASPix  SR FSM control
  const std::intptr_t ATLASPix_CONTROL_BASE_ADDRESS = 0x43C20000;
  const std::size_t ATLASPix_CONTROL_MAP_SIZE = 4096;
  const std::uint32_t ATLASPix_RAM_address_MASK = ATLASPix_CONTROL_MAP_SIZE - 1;

  // ATLASPix Pulser Control
  const std::intptr_t ATLASPix_PULSER_BASE_ADDRESS = 0x43C10000;
  const std::size_t ATLASPix_PULSER_MAP_SIZE = 4096;
  const std::uint32_t ATLASPix_PULSER_MASK = ATLASPix_PULSER_MAP_SIZE - 1;

  // ATLASPix Counter Control
  const std::intptr_t ATLASPix_COUNTER_BASE_ADDRESS = 0x43C00000;
  const std::size_t ATLASPix_COUNTER_MAP_SIZE = 4096;
  const std::uint32_t ATLASPix_COUNTER_MASK = ATLASPix_COUNTER_MAP_SIZE - 1;

  // ATLASPix readout
  const std::intptr_t ATLASPix_READOUT_BASE_ADDRESS = 0x43C70000;
  const std::size_t ATLASPix_READOUT_MAP_SIZE = 16 * 4096;
  const std::uint32_t ATLASPix_READOUT_MASK = ATLASPix_READOUT_MAP_SIZE - 1;

//  const std::size_t ATLASPix_RAM_write_enable_MASK = 0xFFFFFFFF;
//  const std::size_t ATLASPix_RAM_content_MASK = 0xFFFFFFFF;
//  const std::size_t ATLASPix_Config_flag_MASK = 0xFFFFFFFF;
//  const std::size_t ATLASPix_RAM_reg_limit_MASK = 0xFFFFFFFF;
//  const std::size_t ATLASPix_RAM_shift_limit_MASK = 0xFFFFFFFF;

// clang-format on
#define ATLASPix_MEMORY                                                                                                     \
  {                                                                                                                         \
    {"data",                                                                                                                \
     memory_map(                                                                                                            \
       ATLASPix_READOUT_BASE_ADDRESS, 0x0, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},      \
      {"fifo_status",                                                                                                       \
       memory_map(                                                                                                          \
         ATLASPix_READOUT_BASE_ADDRESS, 0x4, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},    \
      {"fifo_config",                                                                                                       \
       memory_map(                                                                                                          \
         ATLASPix_READOUT_BASE_ADDRESS, 0x8, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},    \
      {"config2",                                                                                                           \
       memory_map(                                                                                                          \
         ATLASPix_READOUT_BASE_ADDRESS, 0xC, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},    \
      {"trg_cnt",                                                                                                           \
       memory_map(                                                                                                          \
         ATLASPix_READOUT_BASE_ADDRESS, 0x18, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},   \
      {"ram_base",                                                                                                          \
       memory_map(ATLASPix_CONTROL_BASE_ADDRESS,                                                                            \
                  0x0,                                                                                                      \
                  ATLASPix_CONTROL_MAP_SIZE,                                                                                \
                  ATLASPix_RAM_address_MASK,                                                                                \
                  PROT_READ | PROT_WRITE)},                                                                                 \
      {"readout_fsm",                                                                                                       \
       memory_map(                                                                                                          \
         ATLASPix_READOUT_BASE_ADDRESS, 0x10, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK, PROT_READ | PROT_WRITE)},   \
      {"cnt_rst",                                                                                                           \
       memory_map(                                                                                                          \
         ATLASPix_COUNTER_BASE_ADDRESS, 0x10, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK, PROT_READ | PROT_WRITE)},   \
      {"global_reset",                                                                                                      \
       memory_map(                                                                                                          \
         ATLASPix_COUNTER_BASE_ADDRESS, 0x14, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK, PROT_READ | PROT_WRITE)},   \
      {"counter_base",                                                                                                      \
       memory_map(                                                                                                          \
         ATLASPix_COUNTER_BASE_ADDRESS, 0x0, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK, PROT_READ | PROT_WRITE)},    \
      {"pulser_base",                                                                                                       \
       memory_map(                                                                                                          \
         ATLASPix_PULSER_BASE_ADDRESS, 0x0, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK, PROT_READ | PROT_WRITE)},       \
  }

// clang-format off

/** Dictionary for register address/name lookup for ATLASPix
 */
#define ATLASPix_REGISTERS				\
  {						\
    {"unlock", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"BLResPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ThResPix", register_t<>(0x26, 0xFF, false, true, true)},	\
    {"VNPix", register_t<>(0x26, 0xFF, false, true, true)},	\
    {"VNFBPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNFollPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNRegCasc", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VDel", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPComp", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPDAC", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNPix2", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"BLResDig", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNBiasPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPLoadPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNOutPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPVCO", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNVCO", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPDelDclMux", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNDelDclMux", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPDelDcl", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNDelDcl", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPDelPreEmp", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNDelPreEmp", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPDcl", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNDcl", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNLVDS", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNLVDSDel", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPPump", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"nu", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"RO_res_n", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"Ser_res_n", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"Aur_res_n", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"sendcnt", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"resetckdivend", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"maxcycend", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"slowdownend", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"timerend", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ckdivend2", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ckdivend", register_t<>(0x26, 0xFF, false, true, true)},              \
    {"VPRegCasc", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPRamp", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNcompPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPFoll", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNDACPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VPBiasRec", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"VNBiasRec", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"Invert", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"SelEx", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"SelSlow", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"EnPLL", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"TriggerDelay", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"Reset", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ConnRes", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"SelTest", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"SelTestOut", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"BLPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"nu2", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ThPix", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"nu3", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"trigger_mode", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"ro_enable", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"armduration", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"trigger_injection", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"edge_sel", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"trigger_enable", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"busy_when_armed", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"trigger_always_armed", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"t0_enable", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"gray_decode", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"send_fpga_ts", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"filter_hp", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"hw_masking", register_t<>(0x26, 0xFF, false, true, true)},		\
    {"temperature", register_t<>(0b00000101)},		\
    {"t0_out_periodic", register_t<>(0x26, 0xFF, false, true, true)},		\
  }


  const std::string AXI_registers[] = {"trigger_mode","ro_enable","armduration","trigger_injection"
		  ,"edge_sel","edge_sel","trigger_enable","busy_when_armed","trigger_always_armed","t0_enable",
		  "gray_decode","send_fpga_ts","filter_hp","hw_masking","t0_out_periodic",
      "thpix","blpix"};

  // clang-format on

} // namespace caribou

#endif /* DEVICE_ATLASPix_DEFAULTS_H */
