#include "ATLASPixMatrix.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>

#include "ATLASPix_defaults.hpp"
#include "log.hpp"
#include "utils.hpp"

using namespace caribou;

ATLASPixMatrix::ATLASPixMatrix()
    : VoltageDACConfig(std::make_unique<ATLASPix_Config>()), CurrentDACConfig(std::make_unique<ATLASPix_Config>()),
      MatrixDACConfig(std::make_unique<ATLASPix_Config>()) {

	// make sure we have reasonable defaults
	for (auto& row : TDAC) {
		row.fill(0);
	}
	for (auto& row : MASK) {
		row.fill(0);
	}
}

void ATLASPixMatrix::_initializeGlobalParameters() {

  VoltageDACConfig->AddParameter("BLPix", 8, ATLASPix_Config::LSBFirst, std::floor(255 * BLPix / 1.8));
  VoltageDACConfig->AddParameter("nu2", 2, ATLASPix_Config::LSBFirst, nu2);
  VoltageDACConfig->AddParameter("ThPix", 8, ATLASPix_Config::LSBFirst, std::floor(255 * ThPix / 1.8));
  VoltageDACConfig->AddParameter("nu3", 2, ATLASPix_Config::LSBFirst, nu3);

  // DAC Block 1 for DIgital Part
  // AnalogDACs
  CurrentDACConfig->AddParameter("unlock", 4, ATLASPix_Config::LSBFirst, 0b1010); // unlock = x101
  CurrentDACConfig->AddParameter("BLResPix", "5,4,3,1,0,2", 5);
  CurrentDACConfig->AddParameter("ThResPix", "5,4,3,1,0,2", 0);
  CurrentDACConfig->AddParameter("VNPix", "5,4,3,1,0,2", 10);
  CurrentDACConfig->AddParameter("VNFBPix", "5,4,3,1,0,2", 20);
  CurrentDACConfig->AddParameter("VNFollPix", "5,4,3,1,0,2", 10);
  CurrentDACConfig->AddParameter("VNRegCasc", "5,4,3,1,0,2", 20); // hier : VNHitbus
  CurrentDACConfig->AddParameter("VDel", "5,4,3,1,0,2", 10);
  CurrentDACConfig->AddParameter("VPComp", "5,4,3,1,0,2", 20); // hier : VPHitbus
  CurrentDACConfig->AddParameter("VPDAC", "5,4,3,1,0,2", 0);
  CurrentDACConfig->AddParameter("VNPix2", "5,4,3,1,0,2", 0);
  CurrentDACConfig->AddParameter("BLResDig", "5,4,3,1,0,2", 5);
  CurrentDACConfig->AddParameter("VNBiasPix", "5,4,3,1,0,2", 0);
  CurrentDACConfig->AddParameter("VPLoadPix", "5,4,3,1,0,2", 5);
  CurrentDACConfig->AddParameter("VNOutPix", "5,4,3,1,0,2", 5);

  // DigitalDACs
  CurrentDACConfig->AddParameter("VPVCO", "5,4,3,1,0,2", 7);        // 5);//7);
  CurrentDACConfig->AddParameter("VNVCO", "5,4,3,1,0,2", 15);       // 15);
  CurrentDACConfig->AddParameter("VPDelDclMux", "5,4,3,1,0,2", 30); // 30);
  CurrentDACConfig->AddParameter("VNDelDclMux", "5,4,3,1,0,2", 30); // 30);
  CurrentDACConfig->AddParameter("VPDelDcl", "5,4,3,1,0,2", 30);    // 30);
  CurrentDACConfig->AddParameter("VNDelDcl", "5,4,3,1,0,2", 30);    // 30);
  CurrentDACConfig->AddParameter("VPDelPreEmp", "5,4,3,1,0,2", 30); // 30);
  CurrentDACConfig->AddParameter("VNDelPreEmp", "5,4,3,1,0,2", 30); // 30);
  CurrentDACConfig->AddParameter("VPDcl", "5,4,3,1,0,2", 30);       // 30);
  CurrentDACConfig->AddParameter("VNDcl", "5,4,3,1,0,2", 30);       // 30);
  CurrentDACConfig->AddParameter("VNLVDS", "5,4,3,1,0,2", 10);      // 10);
  CurrentDACConfig->AddParameter("VNLVDSDel", "5,4,3,1,0,2", 00);   // 10);
  CurrentDACConfig->AddParameter("VPPump", "5,4,3,1,0,2", 5);       // 5);

  CurrentDACConfig->AddParameter("nu", "1,0", 0);
  CurrentDACConfig->AddParameter("RO_res_n", 1, ATLASPix_Config::LSBFirst, 1);  // 1);  //for fastreadout start set 1
  CurrentDACConfig->AddParameter("Ser_res_n", 1, ATLASPix_Config::LSBFirst, 1); // 1);  //for fastreadout start set 1
  CurrentDACConfig->AddParameter("Aur_res_n", 1, ATLASPix_Config::LSBFirst, 1); // 1);  //for fastreadout start set 1
  CurrentDACConfig->AddParameter("sendcnt", 1, ATLASPix_Config::LSBFirst, 0);   // 0);
  CurrentDACConfig->AddParameter("resetckdivend", "3,2,1,0", 15);               // 2);
  CurrentDACConfig->AddParameter("maxcycend", "5,4,3,2,1,0", 5);                // 10); // probably 0 not allowed
  CurrentDACConfig->AddParameter("slowdownend", "3,2,1,0", 2);                  // 1);
  CurrentDACConfig->AddParameter("timerend", "3,2,1,0", 1); // 8); // darf nicht 0!! sonst werden debug ausgaben verschluckt
  CurrentDACConfig->AddParameter("ckdivend2", "5,4,3,2,1,0", 4); // 1);
  CurrentDACConfig->AddParameter("ckdivend", "5,4,3,2,1,0", 4);  // 1);
  CurrentDACConfig->AddParameter("VPRegCasc", "5,4,3,1,0,2", 20);
  CurrentDACConfig->AddParameter("VPRamp", "5,4,3,1,0,2", 0);     // was 4, off for HB/Thlow usage and fastreadout
  CurrentDACConfig->AddParameter("VNcompPix", "5,4,3,1,0,2", 10); // VNComparator
  CurrentDACConfig->AddParameter("VPFoll", "5,4,3,1,0,2", 10);
  CurrentDACConfig->AddParameter("VNDACPix", "5,4,3,1,0,2", 8);
  CurrentDACConfig->AddParameter("VPBiasRec", "5,4,3,1,0,2", 30);
  CurrentDACConfig->AddParameter("VNBiasRec", "5,4,3,1,0,2", 30);
  CurrentDACConfig->AddParameter("Invert", 1, ATLASPix_Config::LSBFirst, 0);  // 0);
  CurrentDACConfig->AddParameter("SelEx", 1, ATLASPix_Config::LSBFirst, 1);   // 1); //activated external clock input
  CurrentDACConfig->AddParameter("SelSlow", 1, ATLASPix_Config::LSBFirst, 1); // 1);
  CurrentDACConfig->AddParameter("EnPLL", 1, ATLASPix_Config::LSBFirst, 0);   // 0);
  CurrentDACConfig->AddParameter("TriggerDelay", 10, ATLASPix_Config::LSBFirst, 0);
  CurrentDACConfig->AddParameter("Reset", 1, ATLASPix_Config::LSBFirst, 0);
  CurrentDACConfig->AddParameter("ConnRes", 1, ATLASPix_Config::LSBFirst, 1); // 1);   //activates termination for output
                                                                              // lvds
  CurrentDACConfig->AddParameter("SelTest", 1, ATLASPix_Config::LSBFirst, 0);
  CurrentDACConfig->AddParameter("SelTestOut", 1, ATLASPix_Config::LSBFirst, 0);
}

void ATLASPixMatrix::_initializeM1LikeColumnParameters() {
  for(int col = 0; col < ncol; col++) {
    std::string s = to_string(col);
    MatrixDACConfig->AddParameter("RamDown" + s, 4, ATLASPix_Config::LSBFirst, 0b000); // 0b1011
    MatrixDACConfig->AddParameter("colinjDown" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("hitbusDown" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("unusedDown" + s, 2, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("RamUp" + s, 4, ATLASPix_Config::LSBFirst, 0b000); // 0b1011
    MatrixDACConfig->AddParameter("colinjUp" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("hitbusUp" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("unusedUp" + s, 2, ATLASPix_Config::LSBFirst, 0);
  }
}

void ATLASPixMatrix::_initializeM2ColumnParameters() {
  for(int col = 0; col < ndoublecol; col++) {
    std::string s = to_string(col);
    MatrixDACConfig->AddParameter("RamL" + s, 3, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("colinjL" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("RamR" + s, 3, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("colinjR" + s, 1, ATLASPix_Config::LSBFirst, 0);
  }
}

void ATLASPixMatrix::_initializeRowParameters() {
  for(int row = 0; row < nrow; row++) {
    std::string s = to_string(row);
    MatrixDACConfig->AddParameter("writedac" + s, 1, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("unused" + s, 3, ATLASPix_Config::LSBFirst, 0);
    MatrixDACConfig->AddParameter("rowinjection" + s, 1, ATLASPix_Config::LSBFirst, 0);
    if(row == 0) {
      MatrixDACConfig->AddParameter("analogbuffer" + s, 1, ATLASPix_Config::LSBFirst, 0);
    } else {
      MatrixDACConfig->AddParameter("analogbuffer" + s, 1, ATLASPix_Config::LSBFirst, 0);
    }
  }
}

void ATLASPixMatrix::initializeM1() {
  BLPix = 0.8;
  ThPix = 0.85;
  ncol = ncol_m1;
  ndoublecol = ncol_m1 / 2;
  nrow = nrow_m1;
  counter = 2;
  nSRbuffer = 104;
  extraBits = 16;
  SRmask = 0x2;
  PulserMask = 0x2;
  flavor = ATLASPix1Flavor::M1;
  GNDDACPix = ATLASPix_GndDACPix_M1;
  VMINUSPix = ATLASPix_VMinusPix_M1;
  GatePix = ATLASPix_GatePix_M1;

  _initializeGlobalParameters();
  _initializeM1LikeColumnParameters();
  _initializeRowParameters();
}

void ATLASPixMatrix::initializeM1Iso() {

  BLPix = 0.8;
  ThPix = 0.86 + 0.014;
  ncol = ncol_m1iso;
  ndoublecol = ncol_m1iso / 2;
  nrow = nrow_m1iso;
  counter = 1;
  nSRbuffer = 104;
  extraBits = 16;
  SRmask = 0x4;
  PulserMask = 0x4;
  flavor = ATLASPix1Flavor::M1Iso;
  GNDDACPix = ATLASPix_GndDACPix_M1ISO;
  VMINUSPix = ATLASPix_VMinusPix_M1ISO;
  GatePix = ATLASPix_GatePix_M1ISO;

  _initializeGlobalParameters();
  _initializeM1LikeColumnParameters();
  _initializeRowParameters();
}

void ATLASPixMatrix::initializeM2() {

  BLPix = 0.8;
  ThPix = 0.85;
  ncol = ncol_m2;
  ndoublecol = ncol_m2 / 2;
  nrow = nrow_m2;
  counter = 3;
  nSRbuffer = 84;
  extraBits = 0;
  SRmask = 0x1;
  PulserMask = 0x1;
  flavor = ATLASPix1Flavor::M2;
  GNDDACPix = ATLASPix_GndDACPix_M2;
  VMINUSPix = ATLASPix_VMinusPix_M2;
  GatePix = ATLASPix_GatePix_M2;

  _initializeGlobalParameters();
  _initializeM2ColumnParameters();
  _initializeRowParameters();
}

void ATLASPixMatrix::setTDAC(uint32_t col, uint32_t row, uint32_t value) {
  if(7 < value) {
    LOG(logWARNING) << "TDAC value out of range, setting to 7";
    value = 7;
  }

  TDAC[col][row] = (value << 1) | MASK[col][row];
}

void ATLASPixMatrix::setUniformTDAC(uint32_t value) {
  if(7 < value) {
    LOG(logWARNING) << "TDAC value out of range, setting to 7";
    value = 7;
  }

  for(int col = 0; col < ncol; col++) {
    for(int row = 0; row < nrow; row++) {
      MASK[col][row] = 0;
      TDAC[col][row] = (value << 1);
    }
  }
}

void ATLASPixMatrix::setMask(uint32_t col, uint32_t row, uint32_t value) {
  MASK[col][row] = (value & 0x1);
  TDAC[col][row] = TDAC[col][row] | (value & 0x1);
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
      out << std::left << std::setw(2) << (TDAC[col][row] >> 1) << " ";
      out << std::left << std::setw(1) << MASK[col][row] << std::endl;
    }
  }
}

void ATLASPixMatrix::loadTDAC(std::string filename) {
  std::ifstream cfg(filename, std::ifstream::in);

  while(cfg) {
    uint32_t col, row, TDAC, mask;
    cfg >> col >> row >> TDAC >> mask;

    setMask(col, row, mask);
    setTDAC(col, row, TDAC);
  }
}

std::vector<uint32_t> ATLASPixMatrix::encodeShiftRegister() const {
  // encode all dacs in a single large bit vector
  auto volBits = VoltageDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
  auto curBits = CurrentDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
  auto matBits = MatrixDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
  std::vector<bool> bits;
  bits.insert(bits.end(), volBits.begin(), volBits.end());
  bits.insert(bits.end(), curBits.begin(), curBits.end());
  bits.insert(bits.end(), matBits.begin(), matBits.end());
  bits.insert(bits.end(), curBits.begin(), curBits.end());

  // encode bitstream into list of 32bit words
  std::vector<uint32_t> words;
  uint32_t buffer = 0;
  size_t cnt = 0;
  for(auto i = bits.begin(); i != bits.end(); ++i) {
    if(cnt == 32) {
      words.push_back(buffer);
      buffer = 0;
      cnt = 0;
    }
    buffer += *i << cnt;
    cnt += 1;
  }
  // loop adds words only after crossing a word boundary
  // last word needs to be added manually
  words.push_back(buffer);

  // verify with configuration values
  if((cnt % 32) != extraBits) {
    LOG(logERROR) << "Encoded shift register extra bits " << cnt << " inconsistent with expected bits " << extraBits;
  }
  // nSRbuffer counts the number of full buffer words
  size_t expectedWords = (cnt == 32) ? nSRbuffer : (nSRbuffer + 1);
  if(words.size() != expectedWords) {
    LOG(logERROR) << "Encoded shift register size " << words.size() << " inconsistent with expected size " << expectedWords;
  }

  return words;
}
