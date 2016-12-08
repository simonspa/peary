/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#define DEVICE_NAME "CLCIpix2"

#include "device.h"
#include "configuration.h"
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
  clicpix2() : caribouDevice(caribou::Configuration()) {};
    ~clicpix2() {};

    /** Initializer function for CLICpix2
     */
    void init();

    /** Turn on the power supply for the CLICpix2 chip
     */
    void powerOn() {};

    /** Turn off the CLICpix2 power
     */
    void powerOff() {};

    /** Start the data acquisition
     */
    void daqStart() {};

    /** Stop the data acquisition
     */
    void daqStop() {};

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
  };

  extern "C" {
    caribouDevice * generator();
  }

} //namespace caribou

#endif /* DEVICE_CLICPIX2_H */
