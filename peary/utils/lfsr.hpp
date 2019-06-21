/* This file contains helper classes which are used by the Caribou
 * library implementations
 */

#ifndef CARIBOU_LFSR_H
#define CARIBOU_LFSR_H

#include <cstdint>

namespace caribou {

  class LFSR {
  public:
    /**
     * Lookup Table for 5-bit XNOR LFSR counters
     */
    static uint16_t LUT13(uint16_t);

    /**
     * Lookup Table for 8-bit XNOR LFSR counters
     */
    static uint8_t LUT8(uint8_t);

    /**
     * Lookup Table for 13-bit XNOR LFSR counters
     */
    static uint8_t LUT5(uint8_t);

  private:
    static const uint16_t lfsr13_lut[8191];
    static const uint8_t lfsr8_lut[255];
    static const uint8_t lfsr5_lut[31];
  };

} // namespace caribou

#endif /* CARIBOU_LFSR_H */
