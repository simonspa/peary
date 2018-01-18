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
#include <string>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>


struct ATLASPixMatrix {

    ATLASPix_Config *CurrentDACConfig;
    ATLASPix_Config *MatrixDACConfig;
    ATLASPix_Config *VoltageDACConfig;

    //global DACs
    uint32_t unlock,BLResPix,ThResPix,VNPix,VNFBPix,VNFollPix,VNRegCasc,VDel,VPComp,VPDAC,VNPix2,BLResDig,VNBiasPix,VPLoadPix,VNOutPix;
    //Digital
    uint32_t  VPVCO,VNVCO,VPDelDclMux,VNDelDclMux,VPDelDcl,VNDelDcl,VPDelPreEmp,VNDelPreEmp,VPDcl,VNDcl,VNLVDS,VNLVDSDel,VPPump,nu,RO_res_n,Ser_res_n,Aur_res_n,sendcnt,
				resetckdivend,maxcycend,slowdownend,timerend,ckdivend2,ckdivend,VPRegCasc,VPRamp,VNcompPix,VPFoll,VNDACPix,VPBiasRec,VNBiasRec,Invert,SelEx,SelSlow,EnPLL,
				TriggerDelay,Reset,ConnRes,SelTest,SelTestOut;

    //Voltage DACs
    double BLPix; // Voltage, to be translated to DAC value
    uint32_t nu2;
    double ThPix; // Voltage, to be translated to DAC value
    uint32_t nu3;
    double VMINUSPix,GatePix,GNDDACPix;


    //Shift register vectors
    std::vector<bool> CurrentDACbits;
    std::vector<bool> MatrixBits;
    std::vector<bool> VoltageDACBits;
    std::vector<uint32_t> Registers;

    //TDAC and mask maps
    std::array<std::array<int, 400>, 56> TDAC;
    std::array<std::array<int, 400>, 56> MASK;

    //info about matrix, SR etc...
    int ncol,nrow,ndoublecol;
    int nSRbuffer,nbits;
    int counter;
    uint32_t SRmask,extraBits,PulserMask;
    int type;
    std::string regcase;


};


namespace caribou {

  /** ATLASPix Device class definition
   */
  class ATLASPix : public pearyDevice<iface_i2c> {

  public:
    ATLASPix(const caribou::Configuration config);
    ~ATLASPix();

    void SetMatrix(std::string matrix);

    std::string getName();

    void printBits(size_t const size, void const * const ptr);

    /** Initializer function for ATLASPix
     */
    void configure();

    void lock();
    void unlock();
    void setThreshold(double threshold);
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

    pearydata getData();


    void Initialize_SR(ATLASPixMatrix *matrix);
    void Shift_SR(ATLASPixMatrix *matrix);
    void Fill_SR(ATLASPixMatrix *matrix);
    void ProgramSR(ATLASPixMatrix *matrix);
    void initTDAC(ATLASPixMatrix *matrix,uint32_t value);
    void setOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value);
    void writePixelInj(ATLASPixMatrix *matrix, uint32_t inj_col, uint32_t inj_row,bool ana_state,bool hb_state);
    void writeOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value);
    void writeAllTDAC(ATLASPixMatrix *matrix);
    void writeUniformTDAC(ATLASPixMatrix *matrix,uint32_t value);
    void loadAllTDAC(std::string filename);
    void LoadTDAC(std::string filename);
    void setAllTDAC(uint32_t value);
    void setMaskPixel(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value);
    void ComputeSCurves(ATLASPixMatrix *matrix,double vmax,int nstep, int npulses,int tup,int tdown);
    void tune(ATLASPixMatrix *matrix, double vmax,int nstep, int npulses, bool tuning_verification);
    void TDACScan(std::string basefolder,int VNDAC,int step,double vmin,double vmax,uint32_t npulses,uint32_t npoints);

    void WriteConfig(std::string filename);
    void LoadConfig(std::string filename);



    void doSCurve(uint32_t col,uint32_t row,double vmin,double vmax,uint32_t npulses,uint32_t npoints);
    void doSCurves(double vmin,double vmax,uint32_t npulses,uint32_t npoints);
    void doSCurves(std::string basefolder,double vmin,double vmax,uint32_t npulses,uint32_t npoints);


    void doNoiseCurve(uint32_t col,uint32_t row);

    void resetPulser();
    void setPulse(ATLASPixMatrix *matrix,uint32_t npulse,uint32_t n_up,uint32_t n_down,double voltage);
    void sendPulse();
    void resetCounters();

    void pulse(uint32_t npulse,uint32_t tup,uint32_t tdown,double amplitude);

    // Setting Special Register
    void setSpecialRegister(std::string name, uint32_t value);


    int readCounter(ATLASPixMatrix *matrix);

    void SetPixelInjection(uint32_t col, uint32_t row,bool ana_state,bool hb_state);
    void SetPixelInjection(ATLASPixMatrix *matrix,uint32_t col, uint32_t row,bool ana_state,bool hb_state);
  private:
    // analog power supply
    // digital power supply

    // hv bias

    // I2C interface
    // reset signal pin
    // power enable pin

    // Access to FPGA memory mapped registers
    int memfd;

    int pulse_width;

    ATLASPixMatrix *theMatrix;

    ATLASPixMatrix *simpleM1;
    ATLASPixMatrix *simpleM1ISO;
    ATLASPixMatrix *simpleM2;

//    ATLASPix_Config *CurrentDACConfig_M1;
//    ATLASPix_Config *MatrixDACConfig_M1;
//    ATLASPix_Config *VoltageDACConfig_M1;
//
//    ATLASPix_Config *CurrentDACConfig_M1ISO;
//    ATLASPix_Config *MatrixDACConfig_M1ISO;
//    ATLASPix_Config *VoltageDACConfig_M1ISO;
//
//    ATLASPix_Config *CurrentDACConfig_M2;
//    ATLASPix_Config *MatrixDACConfig_M2;
//    ATLASPix_Config *VoltageDACConfig_M2;
//
//    //ATLASPix M2 Registers
//    //Analog
//    uint32_t unlock,BLResPix,ThResPix,VNPix,VNFBPix,VNFollPix,VNRegCasc,VDel,VPComp,VPDAC,VNPix2,BLResDig,VNBiasPix,VPLoadPix,VNOutPix;
//    //Digital
//    uint32_t  VPVCO,VNVCO,VPDelDclMux,VNDelDclMux,VPDelDcl,VNDelDcl,VPDelPreEmp,VNDelPreEmp,VPDcl,VNDcl,VNLVDS,VNLVDSDel,VPPump,nu,RO_res_n,Ser_res_n,Aur_res_n,sendcnt,
//				resetckdivend,maxcycend,slowdownend,timerend,ckdivend2,ckdivend,VPRegCasc,VPRamp,VNcompPix,VPFoll,VNDACPix,VPBiasRec,VNBiasRec,Invert,SelEx,SelSlow,EnPLL,
//				TriggerDelay,Reset,ConnRes,SelTest,SelTestOut;
//
//    //Column registers
//    uint32_t ramL[38]={};
//    uint32_t colInjL[38]={};
//    uint32_t ramR[38]={};
//    uint32_t colInjR[38]={};
//
//    // Row registers
//
//    uint32_t writedac[320]={};
//    uint32_t unused[320]={};
//    uint32_t rowinjection[320]={};
//    uint32_t analogbuffer[320]={};
//
//    // Voltage DACs
//
//    double BLPix; // Voltage, to be translated to DAC value
//    uint32_t nu2;
//    double ThPix; // Voltage, to be translated to DAC value
//    uint32_t nu3;
//
//    std::vector<bool> CurrentDACbits_M1;
//    std::vector<bool> MatrixBits_M1;
//    std::vector<bool> VoltageDACBits_M1;
//    std::vector<uint32_t> Registers_M1;
//
//    std::vector<bool> CurrentDACbits_M1ISO;
//    std::vector<bool> MatrixBits_M1ISO;
//    std::vector<bool> VoltageDACBits_M1ISO;
//    std::vector<uint32_t> Registers_M1ISO;
//
//    std::vector<bool> CurrentDACbits_M2;
//    std::vector<bool> MatrixBits_M2;
//    std::vector<bool> VoltageDACBits_M2;
//    std::vector<uint32_t> Registers_M2;
//
//    //pixel dac Matrix 1
//    std::array<std::array<int, ncol_m1>, nrow_m1> M1_TDAC;
//    std::array<std::array<int, ncol_m1>, nrow_m1> M1_MASK;
//
//    //pixel dac Matrix 1 ISO
//    std::array<std::array<int, ncol_m1iso>, nrow_m1iso> M1ISO_TDAC;
//    std::array<std::array<int, ncol_m1iso>, nrow_m1iso> M1ISO_MASK;
//
//    //pixel dac Matrix 2
//    std::array<std::array<int, ncol_m2>, nrow_m2> M2_TDAC;
//    std::array<std::array<int, ncol_m2>, nrow_m2> M2_MASK;
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
