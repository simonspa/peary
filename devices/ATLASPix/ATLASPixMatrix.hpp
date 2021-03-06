#ifndef DEVICE_ATLASPIXMATRIX_H
#define DEVICE_ATLASPIXMATRIX_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ATLASPix_Config.hpp"

enum class ATLASPix1Flavor { M1, M1Iso, M2, Undefined };

/** ATLASPix matrix configuration and related methods.
 *
 * Handles the shift register encoding and configuration files.
 */
struct ATLASPixMatrix {
  std::unique_ptr<ATLASPix_Config> VoltageDACConfig;
  std::unique_ptr<ATLASPix_Config> CurrentDACConfig;
  std::unique_ptr<ATLASPix_Config> MatrixDACConfig;

  // Voltage DACs
  double BLPix; // Voltage, to be translated to DAC value
  uint32_t nu2;
  double ThPix; // Voltage, to be translated to DAC value
  uint32_t nu3;
  double VMINUSPix, GatePix, GNDDACPix, VMinusPD, VNFBPix, VMain2, BLResPix;

  // TDAC and mask maps
  std::array<std::array<int, 400>, 56> TDAC; // last bit also encodes mask
  std::array<std::array<int, 400>, 56> MASK; // duplicate of last TDAC bit

  // info about matrix, SR etc...
  uint32_t ncol, nrow, ndoublecol;
  uint32_t nSRbuffer, nbits;
  int counter;
  uint32_t SRmask, extraBits, PulserMask;
  ATLASPix1Flavor flavor = ATLASPix1Flavor::Undefined;
  int maskx, masky;

  ATLASPixMatrix();

  // initialize for M1 flavor
  void initializeM1();
  // initialize for M1Iso flavor
  void initializeM1Iso();
  // initialize for M2 flavor
  void initializeM2();
  void _initializeGlobalParameters();
  void _initializeM1LikeColumnParameters();
  void _initializeM2ColumnParameters();
  void _initializeRowParameters();

  void setTDAC(uint32_t col, uint32_t row, uint32_t value);
  void setUniformTDAC(uint32_t value);
  void setMask(uint32_t col, uint32_t row, uint32_t value);

  /// Write global configuration file
  void writeGlobal(std::string basename) const;
  /// Write per-pixel trim dac configuration file
  void writeTDAC(std::string basename) const;
  /// Load global configuration file
  void loadGlobal(std::string basename);
  /// Load per-pixel trim dac configuration file
  void loadTDAC(std::string basename);

  /// encode shift register content as vector of 32bit words
  std::vector<uint32_t> encodeShiftRegister() const;
};

#endif // DEVICE_ATLASPIXMATRIX_H
