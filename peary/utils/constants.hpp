/**
 * peary hardware constants
 */

#ifndef CARIBOU_CONSTANTS_H
#define CARIBOU_CONSTANTS_H

/** CaR board I2C device mapping and addresses
 */

#define ADDR_I2CSWTCH 0x74 // PCA9846PW I2C bus switch

// FIXME this mapping depends on the hardware configuration and has to eb adapted somehow
#define I2C0 "/dev/i2c-6"
#define I2C1 "/dev/i2c-7"
#define I2C2 "/dev/i2c-8"
#define I2C3 "/dev/i2c-9"

/** Devices on I2C0
 */
#define ADDR_BRIDGE  0x28 // SC18IS602B BRIDGE SPI/I2C
#define ADDR_EEPROM  0xA0 // 24LC32A EEPROM, Board ID, 12bit memory
#define ADDR_IOEXP   0x76 // PCA9539 IO Expander / Power switch
#define ADDR_CLKGEN  0x68 // SI5345 Clock generator / PLL
#define ADDR_TEMP    0x92 // TMP101 Temperature sensor

/** Devices on I2C1
 */
#define ADDR_MONITOR_1 0x40 // INA226 MONITOR PWR/CURR BIDIR
#define ADDR_MONITOR_2 0x41
#define ADDR_MONITOR_3 0x42
#define ADDR_MONITOR_4 0x43
#define ADDR_MONITOR_5 0x44
#define ADDR_MONITOR_6 0x45
#define ADDR_MONITOR_7 0x46
#define ADDR_MONITOR_8 0x4A

/** Devices on I2C2
 */
// SEAF connector

/** Devices on I2C3
 */
#define ADDR_ADC   0x48 // ADS7828 ADC 12BIT 50KSPS 8CH, Analog input range 0-4V
#define ADDR_DAC_0 0x49 // DAC7678 DAC 12BIT I2C OCTAL 24VQFN
#define ADDR_DAC_1 0x4A
#define ADDR_DAC_2 0x4B
#define ADDR_DAC_3 0x4C
#define ADDR_DAC_4 0x4D
#define ADDR_DAC_5 0x4E
#define ADDR_DAC_6 0x4F


/** TMP101 Thermometer Registers Addresses
 */
#define REG_TEMP_TEMP 0x00
#define REG_TEMP_CONF 0x01
#define REG_TEMP_LOW  0x02
#define REG_TEMP_HIGH 0x03


namespace caribou {

  /** Interfaces
   */
  enum IFACE {
    NONE,
    LOOPBACK,
    I2C,
    SPI
  };

} // end namespace caribou

#endif /* CARIBOU_CONSTANTS_H */
