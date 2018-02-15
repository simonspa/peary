#include "ATLASPixMatrix.hpp"

#include "log.hpp"

using namespace caribou;

ATLASPixMatrix::ATLASPixMatrix()
    : CurrentDACConfig(std::make_unique<ATLASPix_Config>()), MatrixDACConfig(std::make_unique<ATLASPix_Config>()),
      VoltageDACConfig(std::make_unique<ATLASPix_Config>()) {}

void ATLASPixMatrix::setMaskPixel(uint32_t col, uint32_t row, uint32_t value) {
  MASK[col][row] = value;
  TDAC[col][row] = TDAC[col][row] << 1 | value;
}

void ATLASPixMatrix::setOneTDAC(uint32_t col, uint32_t row, uint32_t value) {
  if(7 < value) {
    LOG(logWARNING) << "TDAC value out of range, setting to 7";
    value = 7;
  }

  TDAC[col][row] = (value << 1) | MASK[col][row];
}

void ATLASPixMatrix::setAllTDAC(uint32_t value) {
  if(7 < value) {
    LOG(logWARNING) << "TDAC value out of range, setting to 7";
    value = 7;
  }

  for(int col = 0; col < ncol; col++) {
    for(int row = 0; row < nrow; row++) {
      MASK[col][row] = 0;
      TDAC[col][row] = (value << 1) | MASK[col][row];
    }
  }
}
