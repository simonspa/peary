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

    /* Routine to program the pixel matrix via the SPI interface
     *
     * This routine produces a bit matrix (using STL vector<bool>) which can
     * directly be sent to the ASIC via the SPI interface in blocks of 8bit words.
     * Interleaved flipflops for superpixels and column-end interfaces are
     * accounted for.
     * The endianness of the SPI interface is obeyed when sending the data and the
     * columns are flipped accordingly.
     */
    void configureMatrix(std::string filename);

  private:
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
     * Sets the column and row address of the pixel and initializes the latches to zero
     */
    pixelConfig(uint8_t column, uint8_t row) : m_column(column), m_row(row), m_latches(0){};

    /* Mask setting of the pixel
     */
    void SetMask(bool mask) { m_latches &= (mask << 13); }
    bool GetMask() { return (m_latches >> 13) & 0x1; }

    /* Individual threshold adjustment (4bit)
     */
    void SetThreshold(uint8_t thr_adjust) { m_latches &= ((thr_adjust << 8) & 0x0f); }
    uint8_t GetThreshold() { return (m_latches >> 8) & 0x0f; }

    /* Enable/disable counting mode
     */
    void SetCountingMode(bool cntmo) { m_latches &= (cntmo << 3); }
    bool GetCountingMode() { return (m_latches >> 3) & 0x1; }

    /* Enable/disable testpulse circuit for this pixel
     */
    void EnableTestpulse(bool tpen) { m_latches &= (tpen << 2); }
    bool GetEnableTestpulse() { return (m_latches >> 2) & 0x1; }

    /* Enable/disable "long counter" mode (13bit ToA only)
     */
    void LongCounter(bool lgcnt) { m_latches &= (lgcnt << 1); }
    bool GetLongCounter() { return (m_latches >> 1) & 0x1; }

    /* Member function to return single bit of the latches.
     * Used for programming the pixel matrix
     */
    bool GetBit(uint8_t bit) { return ((m_latches >> bit) & 0x1); }

    /* Returns row address
     */
    uint8_t row() const { return m_row; }

    /* Returns column address
     */
    uint8_t column() const { return m_column; }

    bool operator==(const pixelConfig& b) const { return (b.row() == this->row()) && (b.column() == this->column()); };

  private:
    uint8_t m_column;
    uint8_t m_row;
    uint16_t m_latches;
  };

} // namespace caribou

#endif /* DEVICE_CLICPIX2_H */
