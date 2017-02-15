#ifndef DEVICE_C3PD_DEFAULTS_H
#define DEVICE_C3PD_DEFAULTS_H

#include "carboard.hpp"
#include "dictionary.hpp"

namespace caribou {

  /** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
   */
#define DEFAULT_DEVICEPATH BUS_I2C2

  /** Default I2C address for standalone-C3PD board with unconnected I2C address lines
   */
#define C3PD_DEFAULT_I2C 0x50

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

#define C3PD_VDDD     1.8
#define C3PD_VDDD     1.8
#define C3PD_REF      1.0

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


  /** Dictionary for register address/name lookup for C3PD
   */
#define C3PD_DICT			      \
  {					      \
    {"gcr",      registerConfig<>(0x00,255)}, \
    {"isg",      registerConfig<>(0x01,255)}, \
    {"ani",      registerConfig<>(0x02,255)}, \
    {"ano",      registerConfig<>(0x03,255)}, \
    {"vbpre",    registerConfig<>(0x04,255)}, \
    {"vbpcas",   registerConfig<>(0x05,255)}, \
    {"vboalf",   registerConfig<>(0x06,255)}, \
    {"vboahf",   registerConfig<>(0x07,255)}, \
    {"vbls",     registerConfig<>(0x08,255)}, \
    {"vbfbk",    registerConfig<>(0x09,255)}, \
    {"vbs",      registerConfig<>(0x0A,255)}, \
    {"vbsp",     registerConfig<>(0x0B,255)}, \
    {"vbpreoff", registerConfig<>(0x0C,255)}, \
    {"vblsoff",  registerConfig<>(0x0D,255)}, \
    {"vbtp",     registerConfig<>(0x0E,255)}, \
  }

  /** Periphery CaR components for C3PD
   */
#define C3PD_PERIPHERY							\
  {									\
    {"c3pd_vddd", registerConfig<>(ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTC,16)}, \
    {"c3pd_vdda", registerConfig<>(ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTD,16)}, \
    {"c3pd_ref",  registerConfig<>(ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTC,16)}, \
    {"c3pd_ain",  registerConfig<>(ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTA,16)}, \
  }

} //namespace caribou

#endif /* DEVICE_C3PD_DEFAULTS_H */
