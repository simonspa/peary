#include "ATLASPixMatrix.hpp"

#include <algorithm>
#include <fstream>

#include "log.hpp"

using namespace caribou;

ATLASPixMatrix::ATLASPixMatrix()
    : VoltageDACConfig(std::make_unique<ATLASPix_Config>()), CurrentDACConfig(std::make_unique<ATLASPix_Config>()),
      MatrixDACConfig(std::make_unique<ATLASPix_Config>()) {}

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

static const std::vector<std::string> VoltageDACs = {"BLPix", "nu2", "ThPix", "nu3"};
static const std::vector<std::string> CurrentDACs = {
  "unlock",  "BLResPix",      "ThResPix",    "VNPix",        "VNFBPix",   "VNFollPix",   "VNRegCasc",   "VDel",
  "VPComp",  "VPDAC",         "VNPix2",      "BLResDig",     "VNBiasPix", "VPLoadPix",   "VNOutPix",    "VPVCO",
  "VNVCO",   "VPDelDclMux",   "VNDelDclMux", "VPDelDcl",     "VNDelDcl",  "VPDelPreEmp", "VNDelPreEmp", "VPDcl",
  "VNDcl",   "VNLVDS",        "VNLVDSDel",   "VPPump",       "nu",        "RO_res_n",    "Ser_res_n",   "Aur_res_n",
  "sendcnt", "resetckdivend", "maxcycend",   "slowdownend",  "timerend",  "ckdivend2",   "ckdivend",    "VPRegCasc",
  "VPRamp",  "VNcompPix",     "VPFoll",      "VPFoll",       "VNDACPix",  "VPBiasRec",   "VNBiasRec",   "Invert",
  "SelEx",   "SelSlow",       "EnPLL",       "TriggerDelay", "Reset",     "ConnRes",     "SelTest",     "SelTestOut"};
static const std::vector<std::string> ExternalBias = {"BLPix_ext", "ThPix_ext", "VMINUSPix", "GNDDACPix", "GatePix"};

void ATLASPixMatrix::writeGlobal(std::string filename) const {
  std::ofstream cfg(filename, std::ofstream::out | std::ofstream::trunc);

  for(auto const& value : VoltageDACs) {
    cfg << std::left << std::setw(20) << value << " " << VoltageDACConfig->GetParameter(value) << std::endl;
  }
  for(auto const& value : CurrentDACs) {
    cfg << std::left << std::setw(20) << value << " " << CurrentDACConfig->GetParameter(value) << std::endl;
  }
  cfg << std::left << std::setw(20) << "GNDDACPix"
      << " " << GNDDACPix << std::endl;
  cfg << std::left << std::setw(20) << "VMINUSPix"
      << " " << VMINUSPix << std::endl;
  cfg << std::left << std::setw(20) << "GatePix"
      << " " << GatePix << std::endl;
  cfg << std::left << std::setw(20) << "BLPix_ext"
      << " " << BLPix << std::endl;
  cfg << std::left << std::setw(20) << "ThPix_ext"
      << " " << ThPix << std::endl;
}

void ATLASPixMatrix::loadGlobal(std::string filename) {
  std::ifstream cfg(filename, std::ifstream::in);

  while(cfg) {
    std::string reg;
    cfg >> reg;

    LOG(logINFO) << "processing : " << reg;

    if(std::find(ExternalBias.begin(), ExternalBias.end(), reg) != ExternalBias.end()) {
      double bias = 0;
      cfg >> bias;
      if(reg == "VMINUSPix") {
        VMINUSPix = bias;
      } else if(reg == "GNDDACPix") {
        GNDDACPix = bias;
      } else if(reg == "GatePix") {
        GatePix = bias;
      } else if(reg == "BLPix_ext") {
        BLPix = bias;
      } else if(reg == "ThPix_ext") {
        ThPix = bias;
      } else {
        LOG(logERROR) << "unsupported external bias register: " << reg;
      }
    } else if(std::find(CurrentDACs.begin(), CurrentDACs.end(), reg) != CurrentDACs.end()) {
      int value;
      cfg >> value;
      CurrentDACConfig->SetParameter(reg, value);
    } else if(std::find(VoltageDACs.begin(), VoltageDACs.end(), reg) != VoltageDACs.end()) {
      int value;
      cfg >> value;
      VoltageDACConfig->SetParameter(reg, value);
    } else {
      LOG(logERROR) << "unknown register : " << reg;
    }
  }
}

void ATLASPixMatrix::writeTDAC(std::string filename) const {
  std::ofstream out(filename, std::ofstream::out | std::ofstream::trunc);

  for(int col = 0; col < ncol; col++) {
    for(int row = 0; row < nrow; row++) {
      out << std::left << std::setw(3) << col << " ";
      out << std::left << std::setw(3) << row << " ";
      out << std::left << std::setw(2) << TDAC[col][row] << " ";
      out << std::left << std::setw(1) << MASK[col][row] << std::endl;
    }
  }
}

void ATLASPixMatrix::loadTDAC(std::string filename) {
  std::ifstream cfg(filename, std::ifstream::in);

  while(cfg) {
    uint32_t col, row, TDAC, mask;
    cfg >> col >> row >> TDAC >> mask;

    setOneTDAC(col, row, TDAC);
    setMaskPixel(col, row, mask);
  }
}
