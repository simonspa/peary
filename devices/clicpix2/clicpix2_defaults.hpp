#ifndef DEVICE_CLICPIX2_DEFAULTS_H
#define DEVICE_CLICPIX2_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

namespace caribou {

/** Default device path for this device: SPI interface
 */
#define DEFAULT_DEVICEPATH "/dev/spi-0"

#define CLICpix2_VDDD 1.2
#define CLICpix2_VDDD_CURRENT 3
#define CLICpix2_VDDA 1.2
#define CLICpix2_VDDA_CURRENT 3
#define CLICpix2_VDDCML 1.2
#define CLICpix2_VDDCML_CURRENT 3
#define CLICpix2_CMLBUFFERS_VDD 2.5
#define CLICpix2_CMLBUFFERS_VDD_CURRENT 3
#define CLICpix2_CMLBUFFERS_VCCO 1.2
#define CLICpix2_CMLBUFFERS_VCCO_CURRENT 3
#define CLICpix2_CML_IREF 333
#define CLICpix2_CML_IREF_POL PUSH
#define CLICpix2_DAC_IREF 13
#define CLICpix2_DAC_IREF_POL PULL

//CLICpix2 receiver
#define CLICpix2_BASE_ADDRESS 0x43C10000
#define CLICpix2_FIFO_OFFSET 0
#define CLICpix2_COUNTER_OFFSET 4
#define MAP_SIZE 4096
#define MAP_MASK (MAP_SIZE - 1)

  
#define CLICPIX2_REGISTERS                                                                                                  \
  {                                                                                                                         \
      {"readout", register_t<>(0x02)}, \
      {"matrix_programming", register_t<>(0x04)}, {"bias_disc_N", register_t<>(0x0A)}, {"bias_disc_P", register_t<>(0x0C)}, \
      {"ikrum", register_t<>(0x0E)}, {"bias_preamp", register_t<>(0x10)}, {"bias_thadj_DAC", register_t<>(0x12)},           \
      {"bias_buffers_1st", register_t<>(0x14, 0xF0)}, {"bias_buffers_2nd", register_t<>(0x14, 0x0F)},                       \
      {"bias_preamp_casc", register_t<>(0x16)}, {"bias_thadj_casc", register_t<>(0x18)},                                    \
      {"bias_mirror_casc", register_t<>(0x1A)}, {"vfbk", register_t<>(0x1C)}, {"bias_disc_N_OFF", register_t<>(0x1E)},      \
      {"bias_disc_P_OFF", register_t<>(0x20)}, {"bias_preamp_OFF", register_t<>(0x22)},                                     \
      {"threshold_LSB", register_t<>(0x24)}, {"threshold_MSB", register_t<>(0x26)}, {"test_cap_1_LSB", register_t<>(0x28)}, \
      {"test_cap_1_MSB", register_t<>(0x2A)}, {"test_cap_2", register_t<>(0x2C)}, {"output_mux_DAC", register_t<>(0x2E)},   \
                                                                                                                            \
      {"poweron_timer", register_t<>(0x30, 0x3F)}, {"pp_clk_div", register_t<>(0x30, 0xC0)},                                \
                                                                                                                            \
      {"poweroff_timer", register_t<>(0x32, 0x3F)}, {"pp_en", register_t<>(0x32, 0x40)},                                    \
                                                                                                                            \
      {"pulsegen_counts_LSB", register_t<>(0x34)}, {"pulsegen_counts_MSB", register_t<>(0x36, 0x1F)},                       \
      {"pulsegen_delay_LSB", register_t<>(0x38)}, {"pulsegen_delay_MSB", register_t<>(0x3A, 0x1F)},                         \
                                                                                                                            \
      {"gcr_tot_clk_div", register_t<>(0x3C, 0x03)}, {"tot_clk_div", register_t<>(0x3C, 0x03)},                             \
      {"gcr_pol", register_t<>(0x3C, 0x04)}, {"pol", register_t<>(0x3C, 0x04)},                                             \
      {"gcr_bg_tuning", register_t<>(0x3C, 0x38)}, {"bg_tuning", register_t<>(0x3C, 0x38)},                                 \
      {"gcr_bg_en", register_t<>(0x3C, 0x40)}, {"bg_en", register_t<>(0x3C, 0x40)},                                         \
      {"gcr_tp_gen_en", register_t<>(0x3C, 0x80)}, {"tp_gen_en", register_t<>(0x3C, 0x80)},                                 \
                                                                                                                            \
      {"rcr_paral_cols", register_t<>(0x3E, 0x03)}, {"paral_cols", register_t<>(0x3E, 0x03)},                               \
      {"rcr_clk_div", register_t<>(0x3E, 0x0C)}, {"clk_div", register_t<>(0x3E, 0x0C)},                                     \
      {"rcr_comp", register_t<>(0x3E, 0x10)}, {"comp", register_t<>(0x3E, 0x10)},                                           \
      {"rcr_sp_comp", register_t<>(0x3E, 0x20)}, {"sp_comp", register_t<>(0x3E, 0x20)},                                     \
  }

#define CLICPIX2_PERIPHERY                                                                                                  \
  {}

} // namespace caribou

#endif /* DEVICE_CLICPIX2_DEFAULTS_H */
