/* This file contains helper classes which are used by the Caribou
 * library implementations
 */

#ifndef CARIBOU_UTILS_H
#define CARIBOU_UTILS_H

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "exceptions.hpp"

namespace caribou {

  /** Delay helper function
   *  Uses usleep() to wait the given time in milliseconds
   */
  void inline mDelay(uint32_t ms) {
    // Wait for the given time in milliseconds:
    usleep(ms * 1000);
  }

  /** Trims the leading and trainling white space from a string
   */
  std::string trim(const std::string& s);

  /** Efficienctly reverse the bit order of 8-bit word
   */
  uint8_t reverseByte(uint8_t byte);

  /** Converts a string to any type.
   * \param x The string to be converted.
   * \param def The default value to be used in case of an invalid string,
   *            this can also be useful to select the correct template type
   *            without having to specify it explicitly.
   * \return An object of type T with the value represented in x, or if
   *         that is not valid then the value of def.
   */
  template <typename T> inline T from_string(const std::string& x) {
    if(x == "")
      throw ConfigInvalidKey("Empty key");
    T ret = T();
    std::istringstream s(x);
    s >> ret;
    char remain = '\0';
    s >> remain;
    if(remain)
      throw caribou::ConfigInvalid("Invalid argument: " + x);
    return ret;
  }

  template <> inline std::string from_string(const std::string& x) {
    if(x == "")
      throw ConfigInvalidKey("Empty key");
    else
      return x;
  }

  template <> int64_t from_string(const std::string& x);

  template <> uint64_t from_string(const std::string& x);

  template <> inline int32_t from_string(const std::string& x) { return static_cast<int32_t>(from_string<int64_t>(x)); }

  template <> inline uint32_t from_string(const std::string& x) { return static_cast<uint32_t>(from_string<uint64_t>(x)); }

  template <> inline uint8_t from_string(const std::string& x) { return static_cast<uint8_t>(from_string<uint64_t>(x)); }

  /** Converts any type to a string.
   * \param x The value to be converted.
   * \return A string representing the passed in parameter.
   */
  template <typename T> inline std::string to_string(const T& x, int digits = 0) {
    std::ostringstream s;
    s << std::setfill('0') << std::setw(digits) << x;
    return s.str();
  }

  template <typename T> inline std::string to_string(const std::vector<T>& x, const std::string& sep, int digits = 0) {
    std::ostringstream s;
    if(x.size() > 0)
      s << to_string(x[0], digits);
    for(size_t i = 1; i < x.size(); ++i) {
      s << sep << to_string(x[i], digits);
    }
    return s.str();
  }

  template <typename T> inline std::string to_string(const std::vector<T>& x, int digits = 0) {
    return to_string(x, ",", digits);
  }

  inline std::string to_string(const std::string& x, int /*digits*/ = 0) { return x; }

  inline std::string to_string(const char* x, int /*digits*/ = 0) { return x; }

  /** Splits string s into elements at delimiter "delim" and returns them as vector
   */
  template <typename T> std::vector<T>& split(const std::string& s, std::vector<T>& elems, char delim) {

    // If the input string is empty, simply return the default elements:
    if(s.empty())
      return elems;

    // Else we have data, clear the default elements and chop the string:
    elems.clear();
    std::stringstream ss(s);
    std::string item;
    T def = T();
    while(std::getline(ss, item, delim)) {
      try {
        elems.push_back(from_string<T>(item));
      } catch(ConfigInvalidKey&) {
        elems.push_back(def);
      }
    }
    return elems;
  }

  /** Return the binary representation of a char as std::string
   */
  template <typename T> std::string to_bit_string(const T data) {
    std::ostringstream stream;
    for(int i = std::numeric_limits<T>::digits - 1; i >= 0; i--) {
      stream << ((data >> i) & 1);
    }
    return stream.str();
  }

  template <typename T> std::string to_hex_string(const T i) {
    std::ostringstream stream;
    stream << std::hex << std::showbase << std::setfill('0') << std::setw(std::numeric_limits<T>::digits / 4) << std::hex
           << static_cast<unsigned long long>(i);
    return stream.str();
  }

  /** Helper function to return a printed list of an integer vector, used to shield
   *  debug code from being executed if debug level is not sufficient
   */
  template <typename T> std::string listVector(std::vector<T> vec, std::string separator = ", ", bool hex = false) {
    std::stringstream os;
    for(auto it : vec) {
      if(hex)
        os << to_hex_string(it);
      else
        os << static_cast<unsigned int>(it);
      os << separator;
    }
    return os.str();
  }

  template <typename T1, typename T2>
  std::string listVector(std::vector<std::pair<T1, T2>> vec, std::string separator = ", ", bool hex = false) {
    std::stringstream os;
    for(auto it : vec) {
      if(hex)
        os << to_hex_string(it.first) << ":" << to_hex_string(it.second);
      else
        os << static_cast<unsigned int>(it.first) << ":" << static_cast<unsigned int>(it.second);
      os << separator;
    }
    return os.str();
  }

  template <typename T1>
  std::string listVector(std::vector<std::pair<std::string, T1>> vec, std::string separator = ", ", bool hex = false) {
    std::stringstream os;
    for(auto it : vec) {
      if(hex)
        os << it.first << ":" << to_hex_string(it.second);
      else
        os << it.first << ":" << static_cast<unsigned int>(it.second);
      os << separator;
    }
    return os.str();
  }

} // namespace caribou

#endif /* CARIBOU_UTILS_H */
