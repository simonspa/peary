/**
 * peary hardware constants
 */

#ifndef CARIBOU_CONSTANTS_H
#define CARIBOU_CONSTANTS_H

/** Firmware Register Addresses
 */
#define ADDR_FW_ID 0x0

/** Memory device of FPGA
 */
#define MEM_PATH "/dev/mem"

namespace caribou {

  /** Interfaces
   */
  enum IFACE {
    NONE,
    LOOPBACK,
    I2C,
    SPI,
    SPI_CLICpix2,
  };

  /** Types of register configurations
   */
  enum REGTYPE {
    UNDEFINED,
    VOLTAGE_SET,
    VOLTAGE_GET,
  };

} // end namespace caribou

#endif /* CARIBOU_CONSTANTS_H */
