/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#include <string>
#include <vector>
#include "Si5345-RevB-CLICpix2-Registers.h"
#include "clicpix2_defaults.hpp"
#include "clicpix2_frameDecoder.hpp"
#include "clicpix2_pixels.hpp"
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
    clicpix2(const caribou::Configuration config);
    ~clicpix2();

    std::string getName();

    /** Initializer function for CLICpix2
     */
    void configure();

    /** Turn on the power supply for the CLICpix2 chip
     */
    void powerUp();

    /** Turn off the CLICpix2 power
     */
    void powerDown();

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
    void configurePatternGenerator(std::string filename);
    void triggerPatternGenerator(bool sleep);

    std::vector<uint32_t> getRawData();
    pearydata getData();

    // Method returns stored timestamps
    std::vector<uint64_t> timestampsPatternGenerator();

    void setSpecialRegister(std::string name, uint32_t value);

    // Reset the chip
    // The reset signal is asserted for ~1us
    void reset();

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

    /* Map of pixelConfigs for configuration storage (row/col)
     */
    std::map<std::pair<uint8_t, uint8_t>, pixelConfig> pixelsConfig;

    // The functions sets clocks required by CLICpix2 to operate
    void configureClock();

    // Methods decodes frame
    pearydata decodeFrame(const std::vector<uint32_t>& frame);

    // Access to FPGA memory mapped registers
    int memfd;

    // CLICpix2 receiver memory map address
    void* receiver_base;

    // CLICpix2 control memory map address
    void* control_base;

    // Total pattern generator length
    uint32_t pg_total_length;
  };

  extern "C" {
  caribouDevice* generator(const caribou::Configuration);
  }

} // namespace caribou

#endif /* DEVICE_CLICPIX2_H */
