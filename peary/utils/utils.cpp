
#include <cstdint>

#include "exceptions.hpp"
#include "utils.hpp"

uint8_t caribou::reverseByte(uint8_t n) {

  /** Lookup table for bit order reversion
   */
  static unsigned char lookup[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
  };

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
  return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}

std::string caribou::trim(const std::string& s) {
  static const std::string spaces = " \t\n\r\v";
  size_t b = s.find_first_not_of(spaces);
  size_t e = s.find_last_not_of(spaces);
  if(b == std::string::npos || e == std::string::npos) {
    return "";
  }
  return std::string(s, b, e - b + 1);
}

template <> int64_t caribou::from_string(const std::string& x) {
  if(x == "")
    throw ConfigInvalidKey("Empty key");
  const char* start = x.c_str();
  size_t end = 0;
  int base = 10;
  std::string bases("box");
  if(x.length() > 2 && x[0] == '0' && bases.find(x[1]) != std::string::npos) {
    if(x[1] == 'b')
      base = 2;
    else if(x[1] == 'o')
      base = 8;
    else if(x[1] == 'x')
      base = 16;
    start += 2;
  }
  int64_t result = static_cast<int64_t>(std::stoll(start, &end, base));
  if(!x.substr(end).empty())
    throw caribou::ConfigInvalid("Invalid argument: " + x);
  return result;
}

template <> uint64_t caribou::from_string(const std::string& x) {
  if(x == "")
    throw ConfigInvalidKey("Empty key");
  const char* start = x.c_str();
  size_t end = 0;
  int base = 10;
  std::string bases("box");
  if(x.length() > 2 && x[0] == '0' && bases.find(x[1]) != std::string::npos) {
    if(x[1] == 'b')
      base = 2;
    else if(x[1] == 'o')
      base = 8;
    else if(x[1] == 'x')
      base = 16;
    start += 2;
  }
  uint64_t result = static_cast<uint64_t>(std::stoull(start, &end, base));
  if(!x.substr(end).empty())
    throw caribou::ConfigInvalid("Invalid argument: " + x);
  return result;
}
