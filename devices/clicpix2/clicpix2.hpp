/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#include <string>
#include <vector>
#include "Si5345-RevB-CLICpix2-Registers.h"
#include "clicpix2_defaults.hpp"
#include "configuration.hpp"
#include "device.hpp"
#include "pearydevice.hpp"
#include "spi_CLICpix2.hpp"

namespace caribou {

  class pixelConfig;

  /** CLICpix2 Device class definition
   *
   *  this class implements the required functionality to operate CLICpix2 chips via the
   *  Caribou device class interface.
   */
  class clicpix2 : public pearyDevice<iface_spi_CLICpix2> {

  public:
    clicpix2(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), 0) {

      // Set up periphery
      _periphery.add("vddd", PWR_OUT_1);
      _periphery.add("vdda", PWR_OUT_3);
      _periphery.add("cmlbuffers_vdd", PWR_OUT_4);
      _periphery.add("vddcml", PWR_OUT_5);
      _periphery.add("cmlbuffers_vcco", PWR_OUT_7);

      _periphery.add("cml_iref", CUR_1);
      _periphery.add("dac_iref", CUR_2);

      _periphery.add("dac_out", VOL_IN_1);

      // Add the register definitions to the dictionary for convenient lookup of names:
      _registers.add(CLICPIX2_REGISTERS);
    };
    ~clicpix2();

    std::string getName();

    /** Initializer function for CLICpix2
     */
    void init();

    /** Turn on the power supply for the CLICpix2 chip
     */
    void powerOn();

    /** Turn off the CLICpix2 power
     */
    void powerOff();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop(){};

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface();

    void configureMatrix(std::string filename);

  private:
    /* Routine to program the pixel matrix via the SPI interface
     *
     * This routine produces a bit matrix (using STL vector<bool>) which can
     * directly be sent to the ASIC via the SPI interface in blocks of 8bit words.
     * Interleaved flipflops for superpixels and column-end interfaces are
     * accounted for.
     * The endianness of the SPI interface is obeyed when sending the data and the
     * columns are flipped accordingly.
     */
    void programMatrix();

    /* Routine to read the pixel matrix configuration from file and store it
     */
    void readMatrix(std::string filename);

    /* Map of pixelConfigs for configuration storage (col/row)
     */
    std::map<std::pair<uint8_t, uint8_t>, pixelConfig> pixels;

    // The functions sets clocks required by CLICpix2 to operate
    void configureClock();
  };

  extern "C" {
  caribouDevice* generator(const caribou::Configuration);
  }

  /* CLICpix2 pixel configuration class
   *
   * Class to hold all information required to fully configure one pixel.
   * The information is internally stored in the same way, the chip stores it, as
   * a 14bit register. The individual values are set via the member functions
   * and can be retrieved bitwise for convenience.
   */
  class pixelConfig {
  public:
    /* Default constructor
     *
     * Initializes the pixel in a masked state
     */
    pixelConfig() : m_latches(0x2000){};
    pixelConfig(bool mask, uint8_t threshold, bool cntmode, bool tpenable, bool longcnt) : pixelConfig() {
      SetMask(mask);
      SetThreshold(threshold);
      SetCountingMode(cntmode);
      EnableTestpulse(tpenable);
      LongCounter(longcnt);
    };

    /* Mask setting of the pixel
     */
    void SetMask(bool mask) {
      if(mask)
        m_latches |= (1 << 13);
      else
        m_latches &= ~(1 << 13);
    }
    bool GetMask() { return (m_latches >> 13) & 0x1; }

    /* Individual threshold adjustment (4bit)
     */
    void SetThreshold(uint8_t thr_adjust) { m_latches = (m_latches & 0xf0ff) | ((thr_adjust & 0x0f) << 8); }
    uint8_t GetThreshold() { return (m_latches >> 8) & 0x0f; }

    /* Enable/disable counting mode
     */
    void SetCountingMode(bool cntmo) {
      if(cntmo)
        m_latches |= (1 << 3);
      else
        m_latches &= ~(1 << 3);
    }
    bool GetCountingMode() { return (m_latches >> 3) & 0x1; }

    /* Enable/disable testpulse circuit for this pixel
     */
    void EnableTestpulse(bool tpen) {
      if(tpen)
        m_latches |= (1 << 2);
      else
        m_latches &= ~(1 << 2);
    }
    bool GetEnableTestpulse() { return (m_latches >> 2) & 0x1; }

    /* Enable/disable "long counter" mode (13bit ToA only)
     */
    void LongCounter(bool lgcnt) {
      if(lgcnt)
        m_latches |= (1 << 1);
      else
        m_latches &= ~(1 << 1);
    }
    bool GetLongCounter() { return (m_latches >> 1) & 0x1; }

    /* Member function to return single bit of the latches.
     * Used for programming the pixel matrix
     */
    bool GetBit(uint8_t bit) { return ((m_latches >> bit) & 0x1); }

    uint16_t GetLatches() { return m_latches; }

    /** Overloaded ostream operator for simple printing of pixel data
     */
    friend std::ostream& operator<<(std::ostream& out, pixelConfig& px) {
      out << "px [" << px.GetMask() << "|" << static_cast<int>(px.GetThreshold()) << "|" << px.GetCountingMode() << "|"
          << px.GetEnableTestpulse() << "|" << px.GetLongCounter() << "]";
      return out;
    }

  private:
    uint16_t m_latches;
  };

} // namespace caribou

#endif /* DEVICE_CLICPIX2_H */
