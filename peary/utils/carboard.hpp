/** CaR board I2C device mapping and addresses
 */

#ifndef CARIBOU_CARBOARD_H
#define CARIBOU_CARBOARD_H

#define ADDR_I2CSWTCH 0x74 // PCA9846PW I2C bus switch

// FIXME this mapping depends on the hardware configuration and has to eb adapted somehow
#define BUS_I2C0 "/dev/i2c-6"
#define BUS_I2C1 "/dev/i2c-7"
#define BUS_I2C2 "/dev/i2c-8"
#define BUS_I2C3 "/dev/i2c-9"

/** Reference voltages
 */
#define CAR_VREF_4P0  4.096 // via TI REF5040


/** Devices on I2C0
 */
#define ADDR_BRIDGE  0x28 // SC18IS602B BRIDGE SPI/I2C
#define ADDR_EEPROM  0xA0 // 24LC32A EEPROM, Board ID, 12bit memory
#define ADDR_IOEXP   0x76 // PCA9539 IO Expander / Power switch
#define ADDR_CLKGEN  0x68 // SI5345 Clock generator / PLL
#define ADDR_TEMP    0x92 // TMP101 Temperature sensor

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
#define ADDR_ADC     0x48 // ADS7828 ADC 12BIT 50KSPS 8CH, Analog input range 0-4V
#define ADDR_DAC_U50 0x49 // DAC7678 DAC 12BIT I2C OCTAL 24VQFN
#define ADDR_DAC_U44 0x4A
#define ADDR_DAC_U47 0x4B
#define ADDR_DAC_U46 0x4C
#define ADDR_DAC_U45 0x4D
#define ADDR_DAC_U48 0x4E
#define ADDR_DAC_U49 0x4F


/** TMP101 Thermometer Registers Addresses
 */
#define REG_TEMP_TEMP 0x00
#define REG_TEMP_CONF 0x01
#define REG_TEMP_LOW  0x02
#define REG_TEMP_HIGH 0x03

/** DAC7678 Registers Addresses
 */
#define REG_DAC_WRITE_CHANNEL  0x00 // Set DAC; last four bit indicate the channel number
#define REG_DAC_UPDATE_CHANNEL 0x10 // Update DAC; last four bit indicate the channel number
#define REG_DAC_LDAC_CHANNEL   0x20 // Set DAC & update all; last four bit indicate the channel number
#define REG_DAC_WRUP_CHANNEL   0x30 // Set & update DAC; last four bit indicate the channel number
#define REG_DAC_POWER          0x40 // Power on/off DAC
#define REG_DAC_CLEAR          0x50 // Write to clear code reg
#define REG_DAC_LDAC           0x60 // Write to LDAC reg
#define REG_DAC_RESET          0x70 // Software reset
#define REG_DAC_MODE_STATIC    0x80 // Internal reference (static mode)
#define REG_DAC_MODE_FLEX      0x90 // Internal reference (flexible mode)

#define REG_DAC_CHANNEL_VOUTA  0x00
#define REG_DAC_CHANNEL_VOUTB  0x01
#define REG_DAC_CHANNEL_VOUTC  0x02
#define REG_DAC_CHANNEL_VOUTD  0x03
#define REG_DAC_CHANNEL_VOUTE  0x04
#define REG_DAC_CHANNEL_VOUTF  0x05
#define REG_DAC_CHANNEL_VOUTG  0x06
#define REG_DAC_CHANNEL_VOUTH  0x07
#define REG_DAC_CHANNEL_ALL    0x0F

#endif /* CARIBOU_CARBOARD_H */
