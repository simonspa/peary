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
#include <string>

#include "exceptions.hpp"

namespace caribou {

  /** Lookup table for bit order reversion
   */
  static unsigned char lookup[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
    0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

  /** Trims the leading and trainling white space from a string
   */
  std::string trim(const std::string &s);

  /** Efficienctly reverse the bit order of 8-bit word
   */
  uint8_t reverseByte(uint8_t byte);

  /** Return the binary representation of a char as std::string
   */
  std::string bitstring(uint8_t data);

  /** Converts a string to any type.
   * \param x The string to be converted.
   * \param def The default value to be used in case of an invalid string,
   *            this can also be useful to select the correct template type
   *            without having to specify it explicitly.
   * \return An object of type T with the value represented in x, or if
   *         that is not valid then the value of def.
   */
  template <typename T>
    inline T from_string(const std::string &x, const T &def = 0) {
    if (x == "")
      return def;
    T ret = def;
    std::istringstream s(x);
    s >> ret;
    char remain = '\0';
    s >> remain;
    if (remain)
      throw caribou::ConfigInvalid("Invalid argument: " + x);
    return ret;
  }

  template <>
    inline std::string
    from_string(const std::string &x, const std::string &def) {
    return x == "" ? def : x;
  }

  template <>
    int64_t from_string(const std::string &x, const int64_t &def);

  template <>
    uint64_t from_string(const std::string &x, const uint64_t &def);

  template <>
    inline int32_t from_string(const std::string &x, const int32_t &def) {
    return static_cast<int32_t>(from_string(x, (int64_t)def));
  }

  template <>
    inline uint32_t from_string(const std::string &x, const uint32_t &def) {
    return static_cast<uint32_t>(from_string(x, (uint64_t)def));
  }

  template <>
    inline uint8_t from_string(const std::string &x, const uint8_t &def) {
    return static_cast<uint8_t>(from_string(x, (int64_t)def));
  }

  
  /** Converts any type to a string.
   * \param x The value to be converted.
   * \return A string representing the passed in parameter.
   */
  template <typename T>
    inline std::string to_string(const T &x, int digits = 0) {
    std::ostringstream s;
    s << std::setfill('0') << std::setw(digits) << x;
    return s.str();
  }

  template <typename T>
    inline std::string to_string(const std::vector<T> &x, const std::string &sep,
				 int digits = 0) {
    std::ostringstream s;
    if (x.size() > 0)
      s << to_string(x[0], digits);
    for (size_t i = 1; i < x.size(); ++i) {
      s << sep << to_string(x[i], digits);
    }
    return s.str();
  }

  template <typename T>
    inline std::string to_string(const std::vector<T> &x, int digits = 0) {
    return to_string(x, ",", digits);
  }

  inline std::string to_string(const std::string &x, int /*digits*/ = 0) {
    return x;
  }

  inline std::string to_string(const char *x, int /*digits*/ = 0) { return x; }

  /** Splits string s into elements at delimiter "delim" and returns them as vector
   */
  template <typename T>
    std::vector<T> &split(const std::string &s, std::vector<T> &elems, char delim) {

    // If the input string is empty, simply return the default elements:
    if (s.empty()) return elems;

    // Else we have data, clear the default elements and chop the string:
    elems.clear();
    std::stringstream ss(s);
    std::string item;
    T def;
    while (std::getline(ss, item, delim)) {
      elems.push_back(from_string(item, def));
    }
    return elems;
  }

  template< typename T >
  std::string to_hex_string(const T i )
  {
    std::stringstream stream;
    stream << std::hex << std::showbase
	   << std::setfill ('0') << std::setw( std::numeric_limits<T>::digits/4  ) 
	   << std::hex << i
	   << std::dec;
    return stream.str();
  }

  /** Helper function to return a printed list of an integer vector, used to shield
   *  debug code from being executed if debug level is not sufficient
   */
  template <typename T>
  std::string listVector(std::vector<T> vec, std::string separator = ", ", bool hex = false) {
    std::stringstream os;
    for(auto it : vec) {
      if(hex) os << to_hex_string(it);
      else os << static_cast<int>(it);
      os << separator;
    }
    return os.str();
  }
  
} //namespace caribou

#endif /* CARIBOU_UTILS_H */

