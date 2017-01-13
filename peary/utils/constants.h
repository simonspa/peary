/**
 * peary hardware constants
 * this file contains DAC definitions and other global constants such
 * as testboard register ids
 */

#ifndef CARIBOU_CONSTANTS_H
#define CARIBOU_CONSTANTS_H

namespace caribou {

  /** Interfaces
   */
  enum IFACE {
    NONE,
    I2C,
    SPI
  };

} // end namespace caribou

#endif /* CARIBOU_CONSTANTS_H */
