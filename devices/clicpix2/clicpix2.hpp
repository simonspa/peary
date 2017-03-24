/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#include "device.hpp"
#include "clicpix2_defaults.hpp"
#include "Si5345-RevB-CLICpix2-Registers.h"
#include "configuration.hpp"
#include <string>
#include <vector>

namespace caribou {

  /** CLICpix2 Device class definition
   *
   *  this class implements the required functionality to operate CLICpix2 chips via the
   *  Caribou device class interface.
   */
  class clicpix2 : public caribouDevice {
    
  public:
  clicpix2(const caribou::Configuration config) : caribouDevice(config) {
    this->initialize(std::string(DEFAULT_DEVICEPATH),0,caribou::dictionary<uint8_t>(CLICPIX2_PERIPHERY));
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
    void daqStart() {};

    /** Stop the data acquisition
     */
    void daqStop() {};

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface();

  private:

    IFACE interface() { return IFACE::SPI_CLICpix2; }
    
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

    //The functions sets clocks required by CLICpix2 to operate
    void configureClock();
  };

  extern "C" {
    caribouDevice * generator(const caribou::Configuration);
  }

} //namespace caribou

#endif /* DEVICE_CLICPIX2_H */
