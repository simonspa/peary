// Defines CLICTD pixels types

#ifndef CLICTD_PIXELS_HPP
#define CLICTD_PIXELS_HPP

#include <ostream>
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  // Basic pixel class
  // The information is internally stored in the same way, the chip stores it, as
  // a 22bit register.
  //
  // The individual values are set via the member functions of a specialized classes
  class clictd_pixel : public virtual pixel {
  public:
    virtual ~clictd_pixel(){};

    // direct latch access
    void setLatches(uint32_t latches) { m_latches = latches; }

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

    uint32_t GetLatches() const { return m_latches; }

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
    clictd_pixel(uint32_t m_latches) : m_latches(m_latches){};
    uint32_t m_latches;
  };

  /* CLICTD pixel configuration class
   *
   * Class to hold all information required to configure one pixel.
   * The information is internally stored in the same way the chip stores it, i.e. as two separate write commands to the
   * pixel latches of 22bits each
   * The individual values are set via the member functions and can be retrieved bitwise for convenience.
   *
   * Layout Stage 2:
   * ( 1 tDAC7[3] tDAC6[3] tDAC5[3] tDAC4[3] tDAC3[3] tDAC2[3] tDAC1[2] 0 )
   *
   * Layout Stage 1:
   * ( 1 tDAC1[1] tDAC0[3] tpEnA[8] mask[8] tpEnD[1] )
   */
  class pixelConfig : public virtual clictd_pixel {
  public:
    /* Default constructor
     *
     * Initializes the pixel configuration with bit 21 always set to '1'
     */
    pixelConfig() : clictd_pixel(0x00200000){};
    pixelConfig(uint32_t latches) : clictd_pixel(latches){};
  };

  /* CLICTD Stage1 pixel configuration class
   *
   * Class to hold all information required to configure one pixel.
   * The information is internally stored in the same way the chip stores it, i.e. as two separate write commands to the
   * pixel latches of 22bits each
   * The individual values are set via the member functions and can be retrieved bitwise for convenience.
   *
   * Layout Stage 2:
   * ( 1 tDAC7[3] tDAC6[3] tDAC5[3] tDAC4[3] tDAC3[3] tDAC2[3] tDAC1[2] 0 )
   *
   * Layout Stage 1:
   * ( 1 tDAC1[1] tDAC0[3] tpEnA[8] mask[8] tpEnD[1] )
   */
  class pixelConfigStage1 : public pixelConfig {
  public:
    /**
     * Constructor for a stage 1 configuration data object:
     * @param mask       Mask bits for analog front ends
     * @param tp_digital Enable digital test pulsing
     * @param tp_analog  Test pulse enable bits for analog front-ends
     * @param thresholds Threshold tuning DAC settings for channels 0-7 (front-ends 2-7 are ignored)
     */
    pixelConfigStage1(uint8_t mask, bool tp_digital, uint8_t tp_analog, std::vector<uint8_t> thresholds) : pixelConfig() {
      SetMask(mask);
      EnableTestpulseDigital(tp_digital);
      EnableTestpulseAnalog(tp_analog);

      if(thresholds.size() >= 2) {
        SetThreshold(0, thresholds.at(0));
        SetThreshold(1, thresholds.at(1));
      } else {
        throw ConfigInvalid("Wrong number of threshold adjustment settings");
      }
    }

    // By default, instantiated as masked
    pixelConfigStage1() : pixelConfig(0x002001FE){};

  private:
    /* Mask setting of the sub-pixels
     */
    void SetMask(uint8_t mask) {
      uint64_t bit_mask = (0xFF << 1); // Bitmask where the analog frontend masks are located
      m_latches = (m_latches & (~bit_mask)) | (mask << 1);
    }

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
      }
    }

    /* Enable/disable testpulse circuit for this pixel
     */
    void EnableTestpulseDigital(bool tpen) {
      if(tpen)
        m_latches |= 0x1;
      else
        m_latches &= ~0x1;
    }
    /* Enable/disable testpulse circuit for this pixel
    */
    void EnableTestpulseAnalog(uint8_t tpen) {
      uint64_t bit_mask = (0xFF << 9); // Bitmask where the analog testpulse bits are located
      m_latches = (m_latches & (~bit_mask)) | (tpen << 9);
    }

  public:
    uint8_t GetMask() const { return (m_latches >> 1) & 0xFF; }
    bool GetEnableTestpulseDigital() const { return m_latches & 0x1; }
    uint8_t GetEnableTestpulseAnalog() const { return (m_latches >> 9) & 0xFF; }

    /** Overloaded print function for ostream operator
     */
    void print(std::ostream& out) const {
      out << "px [" << this->GetMask() << "|" << this->GetEnableTestpulseDigital() << "-" << this->GetEnableTestpulseAnalog()
          << "|xxxxxxxx]";
    }
  };

  /* CLICTD Stage2 pixel configuration class
   *
   * Layout Stage 2:
   * ( 1 tDAC7[3] tDAC6[3] tDAC5[3] tDAC4[3] tDAC3[3] tDAC2[3] tDAC1[2] 0 )
   */
  class pixelConfigStage2 : public pixelConfig {
  public:
    /**
     * Constructor for a stage 2 configuration data object:
     * @param thresholds Threshold tuning DAC settings for channels 0-7 (front-end 0 is ignored)
     */
    pixelConfigStage2(std::vector<uint8_t> thresholds) : pixelConfig() {
      if(thresholds.size() < 7 || thresholds.size() > 8) {
        throw ConfigInvalid("Wrong number of threshold adjustment settings");
      }

      // Ignore the very first entry if all eicht are shipped:
      for(size_t frontend = 1; frontend < thresholds.size(); frontend++) {
        SetThreshold(frontend, thresholds.at(frontend));
      }
    };

    pixelConfigStage2() : pixelConfig(){};

  private:
    void SetThreshold(uint8_t frontend, uint8_t threshold) {
      // Strip everything but three bits:
      auto value = threshold & 0x07;
      if(frontend == 1) {
        uint64_t bit_mask = (0x06 << 1);
        m_latches = (m_latches & (~bit_mask)) | ((value & 0x06) << 1);
      } else {
        size_t shift = (frontend - 1) * 3;
        uint64_t bit_mask = (0x07 << shift);
        m_latches = (m_latches & (~bit_mask)) | (value << shift);
      }
    }

  public:
    uint8_t GetThreshold(uint8_t frontend) const {
      if(frontend >= 8 || frontend <= 1) {
        throw ConfigInvalid("Frontend does not exist: " + std::to_string(frontend));
      }
      return (m_latches >> ((frontend - 1) * 3)) & 0x07;
    }
    std::vector<uint8_t> GetThresholds() const {
      std::vector<uint8_t> thresholds;
      for(size_t frontend = 2; frontend < 8; frontend++) {
        thresholds.push_back(GetThreshold(frontend));
      }
      return thresholds;
    }

    /** Overloaded print function for ostream operator
     */
    void print(std::ostream& out) const { out << "px [/|/-//|xx" << listVector(this->GetThresholds()) << "]"; }
  };
} // namespace caribou

#endif
