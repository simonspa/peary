
#include <cstdint>

#include "utils.h"

uint8_t caribou::reverseByte(uint8_t n) {
  /** Reverse the top and bottom nibble then swap them:
   *   + lookup reverse of bottom nibble
   *   |       + grab bottom nibble
   *   |       |        + move bottom result into top nibble
   *   |       |        |     + combine the bottom and top results 
   *   |       |        |     | + lookup reverse of top nibble
   *   |       |        |     | |       + grab top nibble
   *   V       V        V     V V       V
   *  (lookup[n&0b1111] << 4) | lookup[n>>4]
   */
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

std::string caribou::trim(const std::string &s) {
  static const std::string spaces = " \t\n\r\v";
  size_t b = s.find_first_not_of(spaces);
  size_t e = s.find_last_not_of(spaces);
  if (b == std::string::npos || e == std::string::npos) {
    return "";
  }
  return std::string(s, b, e - b + 1);
}

