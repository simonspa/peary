/**
 * Caribou implementation for the ATLASPixF chip
 */

#ifndef DEVICE_ATLASPix_H
#define DEVICE_ATLASPix_H

#include "ATLASPix_defaults.hpp"
#include "ATLASPix_Config.hpp"
#include "device.hpp"
#include "i2c.hpp"
#include "pearydevice.hpp"
#include <vector>

namespace caribou {

  /** ATLASPix Device class definition
   */
  class ATLASPix : public pearyDevice<iface_i2c> {

  public:
    ATLASPix(const caribou::Configuration config);
    ~ATLASPix();

    std::string getName();

    void printBits(size_t const size, void const * const ptr);

    /** Initializer function for ATLASPix
     */
    void configure();

    /** Turn on the power supply for the ATLASPix chip
     */
    void powerUp();

    /** Turn off the ATLASPix power
     */
    void powerDown();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface(){};

    // Reset the chip
    // The reset signal is asserted for ~5us
    void reset();


    void LoadConfiguration(int matrix);


    void configureClock();

    void Initialize_SR();
    void Shift_SR();
    void Fill_SR();

    void tune();


    void sendPulse(uint32_t npulse,uint32_t n_up,uint32_t n_down,double voltage);
    void resetCounters();
    int readCounter(int channel);


  private:
    // analog power supply
    // digital power supply

    // hv bias

    // I2C interface
    // reset signal pin
    // power enable pin

    // Access to FPGA memory mapped registers
    int memfd;


    ATLASPix_Config *CurrentDACConfig;
    ATLASPix_Config *MatrixDACConfig;
    ATLASPix_Config *VoltageDACConfig;

    //ATLASPix M2 Registers
    //Analog
    uint32_t unlock,BLResPix,ThResPix,VNPix,VNFBPix,VNFollPix,VNRegCasc,VDel,VPComp,VPDAC,VNPix2,BLResDig,VNBiasPix,VPLoadPix,VNOutPix;
    //Digital
    uint32_t  VPVCO,VNVCO,VPDelDclMux,VNDelDclMux,VPDelDcl,VNDelDcl,VPDelPreEmp,VNDelPreEmp,VPDcl,VNDcl,VNLVDS,VNLVDSDel,VPPump,nu,RO_res_n,Ser_res_n,Aur_res_n,sendcnt,
				resetckdivend,maxcycend,slowdownend,timerend,ckdivend2,ckdivend,VPRegCasc,VPRamp,VNcompPix,VPFoll,VNDACPix,VPBiasRec,VNBiasRec,Invert,SelEx,SelSlow,EnPLL,
				TriggerDelay,Reset,ConnRes,SelTest,SelTestOut;

    //Column registers
    uint32_t ramL[38]={};
    uint32_t colInjL[38]={};
    uint32_t ramR[38]={};
    uint32_t colInjR[38]={};

    // Row registers

    uint32_t writedac[320]={};
    uint32_t unused[320]={};
    uint32_t rowinjection[320]={};
    uint32_t analogbuffer[320]={};

    // Voltage DACs

    double BLPix; // Voltage, to be translated to DAC value
    uint32_t nu2;
    double ThPix; // Voltage, to be translated to DAC value
    uint32_t nu3;

    std::vector<bool> CurrentDACbits;
    std::vector<bool> MatrixBits;
    std::vector<bool> VoltageDACBits;
    std::vector<uint32_t> Registers;

  };

  /** Device generator
   *
   *  This generator function is used by the device manager to create instances of
   *  this device using the abstract caribouDevice class interface. It has to be implemented
   *  for every class deriving from caribouDevice.
   */
  extern "C" {
  caribouDevice* generator(const caribou::Configuration);
  }

} // namespace caribou

#endif /* DEVICE_ATLASPix_H */
