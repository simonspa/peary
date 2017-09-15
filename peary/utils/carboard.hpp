/** CaR board I2C device mapping and addresses
 */

#ifndef CARIBOU_CARBOARD_H
#define CARIBOU_CARBOARD_H

#include "datatypes.hpp"

#define ADDR_I2CSWTCH 0x74 // PCA9846PW I2C bus switch

// FIXME this mapping depends on the hardware configuration and has to be adapted somehow
#define BUS_I2C0 "/dev/i2c-7"
#define BUS_I2C1 "/dev/i2c-8"
#define BUS_I2C2 "/dev/i2c-9"
#define BUS_I2C3 "/dev/i2c-10"

/** Reference voltages, shunts, ...
 */
#define CAR_VREF_4P0 4.096 // via TI REF5040
#define CAR_INA226_R_SHUNT 0.01

// Caribou control addresses
const std::intptr_t CARIBOU_CONTROL_BASE_ADDRESS = 0x43C30000;
const std::intptr_t CARIBOU_FIRMWARE_VERSION_OFFSET = 0;
const std::size_t CARIBOU_CONTROL_MAP_SIZE = 4096;
const std::size_t CARIBOU_CONTROL_MAP_MASK = CARIBOU_CONTROL_MAP_SIZE - 1;

/** Devices on I2C0
 */
#define ADDR_BRIDGE 0x28 // SC18IS602B BRIDGE SPI/I2C
#define ADDR_EEPROM 0xA0 // 24LC32A EEPROM, Board ID, 12bit memory
#define ADDR_IOEXP 0x76  // PCA9539 IO Expander / Power switch
#define ADDR_CLKGEN 0x68 // SI5345 Clock generator / PLL
#define ADDR_TEMP 0x92   // TMP101 Temperature sensor

/** Devices on I2C1
 */
#define ADDR_MONITOR_U53 0x40 // INA226 MONITOR PWR/CURR BIDIR
#define ADDR_MONITOR_U52 0x41
#define ADDR_MONITOR_U55 0x42
#define ADDR_MONITOR_U54 0x43
#define ADDR_MONITOR_U57 0x44
#define ADDR_MONITOR_U56 0x45
#define ADDR_MONITOR_U59 0x46
#define ADDR_MONITOR_U58 0x4A

/** Devices on I2C2
 */
// SEAF connector

/** Devices on I2C3
 */
#define ADDR_ADC 0x48     // ADS7828 ADC 12BIT 50KSPS 8CH, Analog input range 0-4V
#define ADDR_DAC_U44 0x4A // DAC7678 DAC 12BIT I2C OCTAL 24VQFN
#define ADDR_DAC_U45 0x4D
#define ADDR_DAC_U46 0x4C
#define ADDR_DAC_U47 0x4B
#define ADDR_DAC_U48 0x4E
#define ADDR_DAC_U49 0x4F
#define ADDR_DAC_U50 0x49

/** TMP101 Thermometer Registers Addresses
 */
#define REG_TEMP_TEMP 0x00
#define REG_TEMP_CONF 0x01
#define REG_TEMP_LOW 0x02
#define REG_TEMP_HIGH 0x03

/** DAC7678 Registers Addresses
 */
#define REG_DAC_WRITE_CHANNEL 0x00  // Set DAC; last four bit indicate the channel number
#define REG_DAC_UPDATE_CHANNEL 0x10 // Update DAC; last four bit indicate the channel number
#define REG_DAC_LDAC_CHANNEL 0x20   // Set DAC & update all; last four bit indicate the channel number
#define REG_DAC_WRUP_CHANNEL 0x30   // Set & update DAC; last four bit indicate the channel number
#define REG_DAC_POWER 0x40          // Power on/off DAC
#define REG_DAC_CLEAR 0x50          // Write to clear code reg
#define REG_DAC_LDAC 0x60           // Write to LDAC reg
#define REG_DAC_RESET 0x70          // Software reset
#define REG_DAC_MODE_STATIC 0x80    // Internal reference (static mode)
#define REG_DAC_MODE_FLEX 0x90      // Internal reference (flexible mode)

#define REG_DAC_POWERUP 0x00        // Power up DAC(s)
#define REG_DAC_POWERDOWN_1K 0x20   // Power down DAC(s) with 1KOhm to GND
#define REG_DAC_POWERDOWN_100K 0x40 // Power down DAC(s) with 100KOhm to GND
#define REG_DAC_POWERDOWN_HZ 0x60   // Power down DAC(s), Vout is High-Z

#define REG_DAC_CHANNEL_VOUTA 0x00
#define REG_DAC_CHANNEL_VOUTB 0x01
#define REG_DAC_CHANNEL_VOUTC 0x02
#define REG_DAC_CHANNEL_VOUTD 0x03
#define REG_DAC_CHANNEL_VOUTE 0x04
#define REG_DAC_CHANNEL_VOUTF 0x05
#define REG_DAC_CHANNEL_VOUTG 0x06
#define REG_DAC_CHANNEL_VOUTH 0x07
#define REG_DAC_CHANNEL_ALL 0x0F

/** INA226 register addresses
 */
#define REG_ADC_CONFIGURATION 0x00
#define REG_ADC_SHUNT_VOLTAGE 0x01
#define REG_ADC_BUS_VOLTAGE 0x02
#define REG_ADC_POWER 0x03
#define REG_ADC_CURRENT 0x04
#define REG_ADC_CALIBRATION 0x05

/** PCA9539 Output register addresses
 */
// Port 0
#define REG_IOEXP_IO0_0 0x00
#define REG_IOEXP_IO0_1 0x01
#define REG_IOEXP_IO0_2 0x02
#define REG_IOEXP_IO0_3 0x03
#define REG_IOEXP_IO0_4 0x04
#define REG_IOEXP_IO0_5 0x05
#define REG_IOEXP_IO0_6 0x06
#define REG_IOEXP_IO0_7 0x07
// Port 1
#define REG_IOEXP_IO1_0 0x00
#define REG_IOEXP_IO1_1 0x01
#define REG_IOEXP_IO1_2 0x02
#define REG_IOEXP_IO1_3 0x03
#define REG_IOEXP_IO1_4 0x04
#define REG_IOEXP_IO1_5 0x05
#define REG_IOEXP_IO1_6 0x06
#define REG_IOEXP_IO1_7 0x07

/** ADS7828 channel register addresses
 */
#define REG_ADC_CHANNEL_CH0 0x00
#define REG_ADC_CHANNEL_CH1 0x40
#define REG_ADC_CHANNEL_CH2 0x10
#define REG_ADC_CHANNEL_CH3 0x50
#define REG_ADC_CHANNEL_CH4 0x20
#define REG_ADC_CHANNEL_CH5 0x60
#define REG_ADC_CHANNEL_CH6 0x30
#define REG_ADC_CHANNEL_CH7 0x70

const caribou::DCDC_CONVERTER_T LTM_VPWR1("LTM_VPWR1", ADDR_DAC_U49, REG_DAC_CHANNEL_VOUTF);
const caribou::DCDC_CONVERTER_T LTM_VPWR2("LTM_VPWR2", ADDR_DAC_U49, REG_DAC_CHANNEL_VOUTD);
const caribou::DCDC_CONVERTER_T LTM_VPWR3("LTM_VPWR3", ADDR_DAC_U49, REG_DAC_CHANNEL_VOUTB);

const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_1("PWR_OUT_1", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTA, REG_IOEXP_IO1_7, ADDR_MONITOR_U53);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_2("PWR_OUT_2", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTC, REG_IOEXP_IO1_6, ADDR_MONITOR_U52);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_3("PWR_OUT_3", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTE, REG_IOEXP_IO1_5, ADDR_MONITOR_U55);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_4("PWR_OUT_4", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTG, REG_IOEXP_IO1_4, ADDR_MONITOR_U54);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_5("PWR_OUT_5", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTB, REG_IOEXP_IO1_0, ADDR_MONITOR_U57);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_6("PWR_OUT_6", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTD, REG_IOEXP_IO1_1, ADDR_MONITOR_U56);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_7("PWR_OUT_7", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTF, REG_IOEXP_IO1_2, ADDR_MONITOR_U59);
const caribou::VOLTAGE_REGULATOR_T
  PWR_OUT_8("PWR_OUT_8", ADDR_DAC_U50, REG_DAC_CHANNEL_VOUTH, REG_IOEXP_IO1_3, ADDR_MONITOR_U58);

const caribou::CURRENT_SOURCE_T CUR_1("CUR_1", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTB, REG_IOEXP_IO0_6);
const caribou::CURRENT_SOURCE_T CUR_2("CUR_2", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTD, REG_IOEXP_IO0_7);
const caribou::CURRENT_SOURCE_T CUR_3("CUR_3", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTF, REG_IOEXP_IO0_5);
const caribou::CURRENT_SOURCE_T CUR_4("CUR_4", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTH, REG_IOEXP_IO0_4);
const caribou::CURRENT_SOURCE_T CUR_5("CUR_5", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTG, REG_IOEXP_IO0_2);
const caribou::CURRENT_SOURCE_T CUR_6("CUR_6", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTE, REG_IOEXP_IO0_3);
const caribou::CURRENT_SOURCE_T CUR_7("CUR_7", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTC, REG_IOEXP_IO0_1);
const caribou::CURRENT_SOURCE_T CUR_8("CUR_8", ADDR_DAC_U47, REG_DAC_CHANNEL_VOUTA, REG_IOEXP_IO0_0);

const caribou::SLOW_ADC_CHANNEL_T VOL_IN_1("VOL_IN_1", 1, REG_ADC_CHANNEL_CH0);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_2("VOL_IN_2", 2, REG_ADC_CHANNEL_CH1);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_3("VOL_IN_3", 3, REG_ADC_CHANNEL_CH2);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_4("VOL_IN_4", 4, REG_ADC_CHANNEL_CH3);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_5("VOL_IN_5", 5, REG_ADC_CHANNEL_CH4);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_6("VOL_IN_6", 6, REG_ADC_CHANNEL_CH5);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_7("VOL_IN_7", 7, REG_ADC_CHANNEL_CH6);
const caribou::SLOW_ADC_CHANNEL_T VOL_IN_8("VOL_IN_8", 8, REG_ADC_CHANNEL_CH7);

const caribou::BIAS_REGULATOR_T BIAS_1("BIAS_1", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTA);
const caribou::BIAS_REGULATOR_T BIAS_2("BIAS_2", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTC);
const caribou::BIAS_REGULATOR_T BIAS_3("BIAS_3", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTE);
const caribou::BIAS_REGULATOR_T BIAS_4("BIAS_4", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTG);
const caribou::BIAS_REGULATOR_T BIAS_5("BIAS_5", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTB);
const caribou::BIAS_REGULATOR_T BIAS_6("BIAS_6", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTD);
const caribou::BIAS_REGULATOR_T BIAS_7("BIAS_7", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTF);
const caribou::BIAS_REGULATOR_T BIAS_8("BIAS_8", ADDR_DAC_U44, REG_DAC_CHANNEL_VOUTH);

const caribou::BIAS_REGULATOR_T BIAS_9("BIAS_9", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTA);
const caribou::BIAS_REGULATOR_T BIAS_10("BIAS_10", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTC);
const caribou::BIAS_REGULATOR_T BIAS_11("BIAS_11", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTE);
const caribou::BIAS_REGULATOR_T BIAS_12("BIAS_12", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTG);
const caribou::BIAS_REGULATOR_T BIAS_13("BIAS_13", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTB);
const caribou::BIAS_REGULATOR_T BIAS_14("BIAS_14", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTD);
const caribou::BIAS_REGULATOR_T BIAS_15("BIAS_15", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTF);
const caribou::BIAS_REGULATOR_T BIAS_16("BIAS_16", ADDR_DAC_U46, REG_DAC_CHANNEL_VOUTH);

const caribou::BIAS_REGULATOR_T BIAS_17("BIAS_17", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTA);
const caribou::BIAS_REGULATOR_T BIAS_18("BIAS_18", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTC);
const caribou::BIAS_REGULATOR_T BIAS_19("BIAS_19", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTE);
const caribou::BIAS_REGULATOR_T BIAS_20("BIAS_20", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTG);
const caribou::BIAS_REGULATOR_T BIAS_21("BIAS_21", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTB);
const caribou::BIAS_REGULATOR_T BIAS_22("BIAS_22", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTD);
const caribou::BIAS_REGULATOR_T BIAS_23("BIAS_23", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTF);
const caribou::BIAS_REGULATOR_T BIAS_24("BIAS_24", ADDR_DAC_U45, REG_DAC_CHANNEL_VOUTH);

const caribou::BIAS_REGULATOR_T BIAS_25("BIAS_25", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTA);
const caribou::BIAS_REGULATOR_T BIAS_26("BIAS_26", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTC);
const caribou::BIAS_REGULATOR_T BIAS_27("BIAS_27", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTE);
const caribou::BIAS_REGULATOR_T BIAS_28("BIAS_28", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTG);
const caribou::BIAS_REGULATOR_T BIAS_29("BIAS_29", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTB);
const caribou::BIAS_REGULATOR_T BIAS_30("BIAS_30", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTD);
const caribou::BIAS_REGULATOR_T BIAS_31("BIAS_31", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTF);
const caribou::BIAS_REGULATOR_T BIAS_32("BIAS_32", ADDR_DAC_U48, REG_DAC_CHANNEL_VOUTH);

#endif /* CARIBOU_CARBOARD_H */
