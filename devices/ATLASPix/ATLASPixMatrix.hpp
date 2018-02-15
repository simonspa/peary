#ifndef DEVICE_ATLASPIXMATRIX_H
#define DEVICE_ATLASPIXMATRIX_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ATLASPix_Config.hpp"

/** ATLASPix matrix configuration and related methods.
 *
 * Implements configuration settings and encodings.
 */
struct ATLASPixMatrix {
  std::unique_ptr<ATLASPix_Config> CurrentDACConfig;
  std::unique_ptr<ATLASPix_Config> MatrixDACConfig;
  std::unique_ptr<ATLASPix_Config> VoltageDACConfig;

  // global DACs
  uint32_t unlock, BLResPix, ThResPix, VNPix, VNFBPix, VNFollPix, VNRegCasc, VDel, VPComp, VPDAC, VNPix2, BLResDig,
    VNBiasPix, VPLoadPix, VNOutPix;
  // Digital
  uint32_t VPVCO, VNVCO, VPDelDclMux, VNDelDclMux, VPDelDcl, VNDelDcl, VPDelPreEmp, VNDelPreEmp, VPDcl, VNDcl, VNLVDS,
    VNLVDSDel, VPPump, nu, RO_res_n, Ser_res_n, Aur_res_n, sendcnt, resetckdivend, maxcycend, slowdownend, timerend,
    ckdivend2, ckdivend, VPRegCasc, VPRamp, VNcompPix, VPFoll, VNDACPix, VPBiasRec, VNBiasRec, Invert, SelEx, SelSlow, EnPLL,
    TriggerDelay, Reset, ConnRes, SelTest, SelTestOut;

  // Voltage DACs
  double BLPix; // Voltage, to be translated to DAC value
  uint32_t nu2;
  double ThPix; // Voltage, to be translated to DAC value
  uint32_t nu3;
  double VMINUSPix, GatePix, GNDDACPix;

  // Shift register vectors
  std::vector<bool> CurrentDACbits;
  std::vector<bool> MatrixBits;
  std::vector<bool> VoltageDACBits;
  std::vector<uint32_t> Registers;

  // TDAC and mask maps
  std::array<std::array<int, 400>, 56> TDAC;
  std::array<std::array<int, 400>, 56> MASK;

  // info about matrix, SR etc...
  int ncol, nrow, ndoublecol;
  int nSRbuffer, nbits;
  int counter;
  uint32_t SRmask, extraBits, PulserMask;
  int type;
  std::string regcase;

  ATLASPixMatrix();

  void setMaskPixel(uint32_t col, uint32_t row, uint32_t value);
  void setOneTDAC(uint32_t col, uint32_t row, uint32_t value);
  void setAllTDAC(uint32_t value);
};

#endif // DEVICE_ATLASPIXMATRIX_H
