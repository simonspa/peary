#include "ATLASPixMatrix.hpp"

ATLASPixMatrix::ATLASPixMatrix()
    : CurrentDACConfig(std::make_unique<ATLASPix_Config>()), MatrixDACConfig(std::make_unique<ATLASPix_Config>()),
      VoltageDACConfig(std::make_unique<ATLASPix_Config>()) {}
