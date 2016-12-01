/* This file contains helper classes which are used by the Caribou
 * library implementations 
 */

#ifndef CARIBOU_UTILS_H
#define CARIBOU_UTILS_H

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace caribou {

  /** Lookup table for bit order reversion
   */
  static unsigned char lookup[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
    0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

  /** Efficienctly reverse the bit order of 8-bit word
   */
  uint8_t reverseByte(uint8_t byte);

} //namespace caribou

#endif /* CARIBOU_UTILS_H */

