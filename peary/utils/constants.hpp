/**
 * peary hardware constants
 */

#ifndef CARIBOU_CONSTANTS_H
#define CARIBOU_CONSTANTS_H

/** Firmware Register Addresses
 */
#define ADDR_FW_ID 0x0


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
