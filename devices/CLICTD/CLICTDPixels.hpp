// Defines CLICTD pixels types

#ifndef CLICTD_PIXELS_HPP
#define CLICTD_PIXELS_HPP

#include <ostream>
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  // Basic pixel class
  // The information is internally stored in the same way, the chip stores it, as
  // a 44bit register.
  //
  // The individual values are set via the member functions of a specialized classes
  class clictd_pixel : public virtual pixel {
  public:
    virtual ~clictd_pixel(){};

    // direct latch access
    void setLatches(uint64_t latches) { m_latches = latches; }

    // set a dedicated bit of the latch
    void setLatches(bool bit, uint8_t idx) {
      if(bit)
        m_latches |= (1 << idx);
      else
        m_latches &= ~(1 << idx);
    }

    /* Member function to return single bit of the latches.
     */
    bool GetBit(uint8_t bit) { return ((m_latches >> bit) & 0x1); }

    uint16_t GetLatches() const { return m_latches; }

    /** Overloaded comparison operators
     */
    bool operator==(const clictd_pixel& rhs) const {
      if(this->GetLatches() == rhs.GetLatches()) {
        return true;
      }
      return false;
    }

    bool operator!=(const clictd_pixel& rhs) const { return !(*this == rhs); }

  protected:
    // Set default 1-bits:
    clictd_pixel(){};
    clictd_pixel(uint64_t m_latches) : m_latches(m_latches){};
    uint64_t m_latches;
  };

  /* CLICTD pixel configuration class
   *
   * Class to hold all information required to fully configure one pixel.
   * The information is internally stored in the same way, the chip stores it, as
   * a 44bit register. The individual values are set via the member functions
   * and can be retrieved bitwise for convenience.
   *
   * Layout:
   * ( 0 ) ( stage 2 ) ( stage 1 )
   * i.e.
   * ( 0 )
   * ( 1 tDAC7[3] tDAC6[3] tDAC5[3] tDAC4[3] tDAC3[3] tDAC2[3] tDAC1[2] 0 )
   * ( 1 tDAC1[1] tDAC0[3] tpEnA[8] mask[8] tpEnD[1] )
   */
  class pixelConfig : public virtual clictd_pixel {
  public:
    /* Default constructor
     *
     * Initializes the pixel in masked state
     */
    pixelConfig() : clictd_pixel(0x800002001FE){};
    pixelConfig(uint8_t mask, bool tp_digital, uint8_t tp_analog, std::vector<uint8_t> thresholds) : pixelConfig() {
      SetMask(mask);
      EnableTestpulseDigital(tp_digital);
      EnableTestpulseAnalog(tp_analog);
      SetThresholds(thresholds);
    };

    /* Mask setting of the sub-pixels
     */
    void SetMask(uint8_t mask) {
      uint64_t bit_mask = (0xFF << 1); // Bitmask where the analog frontend masks are located
      m_latches = (m_latches & (~bit_mask)) | (mask << 1);
    }
    uint8_t GetMask() const { return (m_latches >> 1) & 0xFF; }

    void SetThreshold(uint8_t frontend, uint8_t threshold) {
      // Strip everything but three bits:
      auto value = threshold & 0x07;
      if(frontend == 0) {
        uint64_t bit_mask = (0x07 << 17);
        m_latches = (m_latches & (~bit_mask)) | (value << 17);
      } else if(frontend == 1) {
        // Least significant bit:
        if(value & 0x1) {
          m_latches |= (0x1 << 20);
        } else {
          m_latches &= ~(0x1 << 20);
        }
        uint64_t bit_mask = (0x06 << 22);
        m_latches = (m_latches & (~bit_mask)) | ((value & 0x06) << 22);
      } else {
        size_t shift = (frontend - 2) * 3 + 25;
        uint64_t bit_mask = (0x07 << shift);
        m_latches = (m_latches & (~bit_mask)) | (value << shift);
      }
    }

    /* Individual threshold adjustment (3bit per analog front-end)
     */
    void SetThresholds(std::vector<uint8_t> thr_adjust) {
      if(thr_adjust.size() != 8) {
        throw ConfigInvalid("Wrong number of threshold adjustments, expecting 8 frontends");
      }

      for(size_t frontend = 0; frontend < 8; frontend++) {
        SetThreshold(frontend, thr_adjust.at(frontend));
      }
    }

    uint8_t GetThreshold(uint8_t frontend) const {
      if(frontend >= 8) {
        throw ConfigInvalid("Frontend does not exist: " + frontend);
      }
      if(frontend == 0) {
        return (m_latches >> 17) & 0x07;
      } else if(frontend == 1) {
        return (m_latches >> 22) & 0x06 || (m_latches >> 20) & 0x01;
      } else {
        return (m_latches >> ((frontend - 2) * 3 + 25)) & 0x07;
      }
    }
    std::vector<uint8_t> GetThresholds() const {
      std::vector<uint8_t> thresholds;
      for(size_t frontend = 0; frontend < 8; frontend++) {
        thresholds.push_back(GetThreshold(frontend));
      }
      return thresholds;
    }

    /* Enable/disable testpulse circuit for this pixel
     */
    void EnableTestpulseDigital(bool tpen) {
      if(tpen)
        m_latches |= 0x1;
      else
        m_latches &= ~0x1;
    }
    bool GetEnableTestpulseDigital() const { return m_latches & 0x1; }

    /* Enable/disable testpulse circuit for this pixel
     */
    void EnableTestpulseAnalog(uint8_t tpen) {
      uint64_t bit_mask = (0xFF << 9); // Bitmask where the analog testpulse bits are located
      m_latches = (m_latches & (~bit_mask)) | (tpen << 9);
    }
    uint8_t GetEnableTestpulseAnalog() const { return (m_latches >> 9) & 0xFF; }

    /** Overloaded print function for ostream operator
     */
    void print(std::ostream& out) const {
      out << "px [" << this->GetMask() << "|" << this->GetEnableTestpulseDigital() << "-" << this->GetEnableTestpulseAnalog()
          << "|" << listVector(this->GetThresholds()) << "]";
    }
  };
} // namespace caribou

#endif
