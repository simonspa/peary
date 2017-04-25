/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#include "device.hpp"
#include "pearydevice.hpp"
#include "spi_CLICpix2.hpp"
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
  class clicpix2 : public pearyDevice<iface_spi_CLICpix2> {
    
  public:
    clicpix2(const caribou::Configuration config) :
      pearyDevice(config, std::string(DEFAULT_DEVICEPATH), 0) {

      // Set up periphery
      _periphery.add("vddd",PWR_OUT_1);
      _periphery.add("vdda",PWR_OUT_3);
      _periphery.add("cmlbuffers_vdd",PWR_OUT_4);
      _periphery.add("vddcml",PWR_OUT_5);
      _periphery.add("cmlbuffers_vcco",PWR_OUT_7);

      _periphery.add("cml_iref",CUR_1);
      _periphery.add("dac_iref",CUR_2);

      _periphery.add("dac_out",VOL_IN_1);

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
    void daqStop() {};

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
    //The functions sets clocks required by CLICpix2 to operate
    void configureClock();
  };

  extern "C" {
    caribouDevice * generator(const caribou::Configuration);
  }

} //namespace caribou

#endif /* DEVICE_CLICPIX2_H */
