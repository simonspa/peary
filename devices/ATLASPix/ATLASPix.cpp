/**
 * Caribou implementation for the ATLASPix
 */

#include "ATLASPix.hpp"

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

uint32_t reverseBits(uint8_t n) {
  uint32_t x = 0;
  for(auto i = 7; n;) {
    x |= (n & 1) << i;
    n >>= 1;
    --i;
  }
  return x;
}

uint32_t reverseBits64(uint64_t n) {
  uint32_t x;
  for(auto i = 63; n;) {
    x |= (n & 1) << i;
    n >>= 1;
    --i;
  }
  return x;
}

// BASIC Configuration

uint32_t grey_decode(uint32_t g) {
  for(uint32_t bit = 1U << 31; bit > 1; bit >>= 1) {
    if(g & bit)
      g ^= bit >> 1;
  }
  return g;
}

pixelhit decodeHit(uint32_t hit) {
  pixelhit tmp;

  tmp.col = (hit >> 25) & 0b11111;

  tmp.row = (hit >> 16) & 0x1FF;
  tmp.ts1 = (hit >> 6) & 0x3FF;
  tmp.ts2 = hit & 0x3F;

  if(((tmp.ts1*2)&0x3F)>tmp.ts2){
	  tmp.tot= 64 - ((tmp.ts1*2)&0x3F) + tmp.ts2;
  }
  else{
	  tmp.tot= + tmp.ts2 - ((tmp.ts1*2)&0x3F);

  }

  return tmp;
}

// pixelhit decodeHit(uint32_t hit, uint32_t TS, uint64_t fpga_ts, uint32_t SyncedTS, uint32_t triggercnt) {
//
//  pixelhit tmp;
//  uint8_t buf = 0;
//
//  tmp.fpga_ts = fpga_ts;
//  tmp.SyncedTS = SyncedTS;
//  tmp.triggercnt = triggercnt;
//  tmp.ATPbinaryCnt = (TS & 0xFFFF00) + grey_decode((TS & 0xFF));
//  tmp.ATPGreyCnt = TS & 0xFF;
//
//  buf = ((hit >> 23) & 0xFF);
//
//  tmp.col = (24 - ((buf >> 2) & 0b00111111));
//  tmp.row = 255 - (reverseBits(hit & 0xFF));
//  tmp.ts2 = (hit >> 18) & 0b00111111;
//  tmp.ts1 = ((hit >> 8) & 0b1111111111); //+ (hit >> 16 & 0b11);
//  tmp.col = tmp.col & 0b11111;
//  tmp.row = tmp.row & 0b111111111;
//
//  //tmp.tot=tmp.ts2<<2;
//
////  gray decoding ToT:
//  int normal = tmp.ts2 & (1 << 5);
//  for(int i=6; i>=0;--i)
//      normal |= (tmp.ts2 ^ (normal >> 1)) & (1 << i);
//  tmp.tot = normal;
//
////  if (tmp.ts2<(tmp.ts1 & 0b111111)){
////	  tmp.tot=64+tmp.ts2;
////  }
////  else{
////	  tmp.tot=tmp.ts2;
////  }
//
//  if((buf >> 1 & 0x1) == 0)
//    tmp.row += 200;
//  return tmp;
//}

namespace Color {
  enum Code {
    FG_DEFAULT = 39,
    BOLD = 1,
    REVERSE = 7,
    RESET = 0,
    FG_BLACK = 30,
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_MAGENTA = 35,
    FG_CYAN = 36,
    FG_LIGHT_GRAY = 37,
    FG_DARK_GRAY = 90,
    FG_LIGHT_RED = 91,
    FG_LIGHT_GREEN = 92,
    FG_LIGHT_YELLOW = 93,
    FG_LIGHT_BLUE = 94,
    FG_LIGHT_MAGENTA = 95,
    FG_LIGHT_CYAN = 96,
    FG_WHITE = 97,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_DEFAULT = 49
  };
  class Modifier {
    Code code;

  public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream& operator<<(std::ostream& os, const Modifier& mod) { return os << "\033[" << mod.code << "m"; }
  };
}

ATLASPix::ATLASPix(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), ATLASPix_DEFAULT_I2C), _daqContinue(ATOMIC_FLAG_INIT) {

  // Configuring the clock
  LOG(logINFO) << "Setting clock circuit on CaR board " << DEVICE_NAME;
  configureClock();

  _registers.add(ATLASPix_REGISTERS);

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);
  volatile uint32_t* pulse_count = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x4);
  volatile uint32_t* high_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x8);
  volatile uint32_t* low_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0xC);
  volatile uint32_t* output_enable =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x10);
  volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);

  *inj_flag = 0x0;
  *pulse_count = 0x0;
  *high_cnt = 0x0;
  *low_cnt = 0x0;
  *output_enable = 0xFFFFFFFF;
  *rst = 0x1;

  // enable data taking
  _daqContinue.test_and_set();
}

ATLASPix::~ATLASPix() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  daqStop(); // does nothing if no daq thread is running
  powerOff();
}

void ATLASPix::SetMatrix(std::string matrix) {

  // avoid stupid case mistakes
  std::string name = matrix;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  // Set up periphery
  _periphery.add("VDDD", PWR_OUT_4);
  _periphery.add("VDDA", PWR_OUT_3);
  _periphery.add("VSSA", PWR_OUT_2);

  if(name == "m1") {

    _periphery.add("GNDDACPix", BIAS_9);
    _periphery.add("VMinusPix", BIAS_5);
    _periphery.add("GatePix", BIAS_2);
    _periphery.add("ThPix", BIAS_25);
    _periphery.add("BLPix", BIAS_17);

    theMatrix.initializeM1();

  } else if(name == "m1iso") {

    _periphery.add("GNDDACPix", BIAS_12);
    _periphery.add("VMinusPix", BIAS_8);
    _periphery.add("GatePix", BIAS_3);
    _periphery.add("ThPix", BIAS_31);
    _periphery.add("BLPix", BIAS_20);

    theMatrix.initializeM1Iso();

  } else if(name == "m2") {

    _periphery.add("GNDDACPix", BIAS_6);
    _periphery.add("VMinusPix", BIAS_4);
    _periphery.add("GatePix", BIAS_1);
    _periphery.add("ThPix", BIAS_28);
    _periphery.add("BLPix", BIAS_23);

    theMatrix.initializeM2();

  } else {
    LOG(logCRITICAL) << "Unknown matrix flavor '" << matrix << "'";
  }
}

void ATLASPix::configure() {

  LOG(logINFO) << "Configuring " << DEVICE_NAME;

  this->resetPulser();
  this->resetCounters();

  // this->powerOn();
  usleep(1000);

  // Build the SR string with default values and shift in the values in the chip
  std::cout << "sending default config " << std::endl;
  this->ProgramSR(theMatrix);

  // this->ComputeSCurves(0,0.5,50,128,100,100);
  std::cout << "sending default TDACs " << std::endl;

  this->writeUniformTDAC(theMatrix, 0b0000);
  this->setSpecialRegister("ro_enable", 0);
  this->setSpecialRegister("armduration", 2000);

  for(int col = 0; col < theMatrix.ncol; col++) {
    for(int row = 0; row < theMatrix.nrow; row++) {
      this->SetPixelInjectionState(col, row, 0, 0, 1);
    }
  }

  this->ProgramSR(theMatrix);
  this->ResetWriteDAC();
  this->ProgramSR(theMatrix);

  for(int col = 0; col < theMatrix.ncol; col++) {
    for(int row = 0; row < theMatrix.nrow; row++) {
      this->SetPixelInjectionState(col, row, 0, 0, 0);
    }
  }

  this->ProgramSR(theMatrix);
  this->ResetWriteDAC();
  this->ProgramSR(theMatrix);

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void ATLASPix::lock() {

  this->theMatrix.CurrentDACConfig->SetParameter("unlock", 0x0);
  this->ProgramSR(theMatrix);
}

void ATLASPix::unlock() {

  this->theMatrix.CurrentDACConfig->SetParameter("unlock", 0b1010);
  this->ProgramSR(theMatrix);
}

void ATLASPix::setThreshold(double threshold) {

  theMatrix.VoltageDACConfig->SetParameter("ThPix", static_cast<int>(floor(255 * threshold / 1.8)));

  this->ProgramSR(theMatrix);

  LOG(logDEBUG) << " ThPix ";
  this->setVoltage("ThPix", threshold);
  this->switchOn("ThPix");
  theMatrix.ThPix = threshold;
}

void ATLASPix::setVMinus(double vminus) {

  // theMatrix.VoltageDACConfig->SetParameter("ThPix",static_cast<int>(floor(255 * threshold/1.8)));

  // this->ProgramSR(theMatrix);
  theMatrix.VMINUSPix = vminus;
  LOG(logDEBUG) << " VMinusPix ";
  this->setVoltage("VMinusPix", vminus);
  this->switchOn("VMinusPix");
}

void ATLASPix::setSpecialRegister(std::string name, uint32_t value) {

  char Choice;

  switch(theMatrix.flavor) {
  case ATLASPix1Flavor::M1:
    Choice = '1';
    break;
  case ATLASPix1Flavor::M1Iso:
    Choice = '3';
    break;
  case ATLASPix1Flavor::M2:
    Choice = '2';
    break;
  default:
    LOG(logERROR) << "Undefined matrix flavor";
  }

  if(name == "unlock") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("unlock", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("unlock", 0x0);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("unlock", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  }

  else if(name == "blrespix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("BLResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("BLResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("BLResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "threspix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("ThResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("ThResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("ThResPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  }

  else if(name == "vnpix") {
    // Set DAC value here calling setParameter
    ////std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    ////std::cin >> Choice ;
    Choice = '1';
    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  }

  else if(name == "vnfbpix") {
    // Set DAC value here calling setParameter
    ////std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    ////std::cin >> Choice ;
    Choice = '1';
    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNFBPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNFBPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNFBPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnfollpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNFollPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNFollPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNFollPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnregcasc") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vdel") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpcomp") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPComp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPComp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPComp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpdac") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPDAC", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPDAC", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPDAC", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnpix2") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNPix2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "blresdig") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("BLResDig", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("BLResDig", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("BLResDig", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnbiaspix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vploadpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPLoadPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPLoadPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPLoadPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnoutpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNOutPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNOutPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNOutPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpvco") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnvco") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNVCO", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpdeldclmux") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vndeldclmux") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDclMux", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpdeldcl") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vndeldcl") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpdelpreemp") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vndelpreemp") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNDelPreEmp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpdcl") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vndcl") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNDcl", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnlvds") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDS", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDS", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDS", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnlvdsdel") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDSDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDSDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNLVDSDel", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vppump") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPPump", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPPump", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPPump", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "nu") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("nu", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("nu", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("nu", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "ro_res_n") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("RO_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("RO_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("RO_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "ser_res_n") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("Ser_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("Ser_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("Ser_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "aur_res_n") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("Aur_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("Aur_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("Aur_res_n", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "sendcnt") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("sendcnt", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("sendcnt", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("sendcnt", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "resetckdivend") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("resetckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("resetckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("resetckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "maxcycend") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("maxcycend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("maxcycend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("maxcycend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "slowdownend") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("slowdownend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("slowdownend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("slowdownend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "timerend") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("timerend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("timerend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("timerend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "ckdivend2") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "ckdivend") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("ckdivend", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpregcasc") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPRegCasc", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpramp") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPRamp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPRamp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPRamp", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vncomppix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNcompPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNcompPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNcompPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpfoll") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPFoll", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPFoll", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPFoll", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vndacpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    ////std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNDACPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNDACPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNDACPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vpbiasrec") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VPBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VPBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VPBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "vnbiasrec") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("VNBiasRec", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "invert") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("Invert", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("Invert", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("Invert", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "selex") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("SelEx", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("SelEx", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("SelEx", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "selslow") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("SelSlow", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("SelSlow", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("SelSlow", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "enpll") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("EnPLL", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("EnPLL", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("EnPLL", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "triggerdelay") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("TriggerDelay", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("TriggerDelay", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("TriggerDelay", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "reset") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("Reset", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("Reset", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("Reset", value);
      this->ProgramSR(theMatrix);
      break;
    }
    default:
      std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
    }
  } else if(name == "connres") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("ConnRes", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("ConnRes", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("ConnRes", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "seltest") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("SelTest", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("SelTest", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("SelTest", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "seltestout") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("SelTestOut", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("SelTestOut", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("SelTestOut", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "blpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.VoltageDACConfig->SetParameter("BLPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.VoltageDACConfig->SetParameter("BLPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.VoltageDACConfig->SetParameter("BLPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "nu2") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("nu2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("nu2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("nu2", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "thpix") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.VoltageDACConfig->SetParameter("ThPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.VoltageDACConfig->SetParameter("ThPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.VoltageDACConfig->SetParameter("ThPix", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  } else if(name == "nu3") {
    // Set DAC value here calling setParameter
    // std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
    // std::cin >> Choice ;

    switch(Choice) {
    case '1': {
      theMatrix.CurrentDACConfig->SetParameter("nu3", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '2': {
      theMatrix.CurrentDACConfig->SetParameter("nu3", value);
      this->ProgramSR(theMatrix);
      break;
    }
    case '3': {
      theMatrix.CurrentDACConfig->SetParameter("nu3", value);
      this->ProgramSR(theMatrix);
      break;
    }
    }
  }

  else if(name == "ro_enable") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
    // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x4);
    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
    // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
    // volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

    *fifo_config = (*fifo_config & 0xFFFFFFFE) + ((value)&0b1);
    //    if(value == 1) {
    //      this->resetCounters();
    //    }

  } else if(name == "trigger_enable") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

    *fifo_config = (*fifo_config & 0xFFFD) + ((value << 1) & 0b10);

  } else if(name == "edge_sel") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
    *fifo_config = (*fifo_config & 0xFFFFFFFB) + ((value << 2) & 0b0100);

  }

  else if(name == "busy_when_armed") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
    *fifo_config = (*fifo_config & 0xFFFFFFF7) + ((value << 3) & 0b1000);
  }

  else if(name == "armduration") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
    // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x4);
    // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x8);
    // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
    volatile uint32_t* config2 = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);

    *config2 = ((value)&0xFFFFFF);

  } else if(name == "trigger_always_armed") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
    // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x4);
    // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x8);
    // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

    *fifo_config = (*fifo_config & 0xFFFFFFBF) + ((value << 6) & 0b1000000);

  }

  else if(name == "t0_enable") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
    // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x4);
    // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x8);
    // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
    volatile uint32_t* fifo_config =
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

    *fifo_config = (*fifo_config & 0xFFFFFF7F) + ((value << 7) & 0b10000000);

  } else if(name == "trigger_injection") {

    void* readout_base =
      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

    // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
    // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x4);
    // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
    // 0x8);
    // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
    volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

    *ro = ((*ro) & 0xFFFFFF) + (value << 24);

  }else if(name == "gray_decode") {

	    void* readout_base =
	      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
	    volatile uint32_t* fifo_config =
	      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
	    *fifo_config = (*fifo_config & 0xFBFF) + ((value << 10) & 0b010000000000);
  }

  else if(name == "ext_clk") {

		    void* readout_base =
		      _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
		    volatile uint32_t* fifo_config =
		      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
		    *fifo_config = (*fifo_config & 0xFDFF) + ((value << 9) & 0b001000000000);
		  }

  else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

void ATLASPix::configureClock() {
/*
  // Check of we should configure for external or internal clock, default to external:
  if(_config.Get<bool>("clock_internal", false)) {
    LOG(logDEBUG) << DEVICE_NAME << ": Configure internal clock source, free running, not locking";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers_free, SI5345_REVB_REG_CONFIG_NUM_REGS_FREE);
    mDelay(100); // let the PLL lock
  } else {
*/
    LOG(logDEBUG) << DEVICE_NAME << ": Configure external clock source, locked to TLU input clock";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
    LOG(logDEBUG) << "Waiting for clock to lock...";

    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        //throw DeviceException("Cannot lock to external clock.");
	break;
    }
    if (_hal->isLockedSI5345()) LOG(logINFO) << "PLL locked to external clock...";
    else LOG(logINFO) << "Cannot lock to external clock, PLL will continue in freerunning mode...";
//  }
}

void ATLASPix::ProgramSR(const ATLASPixMatrix& matrix) {

  auto words = matrix.encodeShiftRegister();

  void* control_base =
    _hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS, ATLASPix_CONTROL_MAP_SIZE, ATLASPix_RAM_address_MASK);
  volatile uint32_t* RAM_address = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x0);
  volatile uint32_t* RAM_content = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x4);
  volatile uint32_t* RAM_write_enable =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x8);
  volatile uint32_t* RAM_reg_limit =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0xC);
  volatile uint32_t* RAM_shift_limit =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x10);
  volatile uint32_t* Config_flag =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x14);
  // volatile uint32_t* global_reset = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) +
  // 0x18);
  volatile uint32_t* output_enable =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x1C);

  *Config_flag = 0;
  *RAM_reg_limit = matrix.nSRbuffer;
  *RAM_shift_limit = matrix.extraBits;

  for(uint32_t i = 0; i <= matrix.nSRbuffer; i++) {
    uint32_t word = words[i];
    *RAM_address = i;
    *RAM_content = word;
    usleep(10);
    *RAM_write_enable = 0x1;
    usleep(10);
    *RAM_write_enable = 0x0;
  };

  *output_enable = matrix.SRmask;
  usleep(10);
  *Config_flag = 0x1;
  usleep(30000);
  *Config_flag = 0;
  *output_enable = 0x0;

  // Sync RO state machine ckdivend with the one in the chip
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);
  *ro = (*ro & 0xFFFFFF00) + (matrix.CurrentDACConfig->GetParameter("ckdivend") & 0xFF);
}

// Injection and pulser

void ATLASPix::resetPulser() {

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);
  usleep(1);
  *rst = 0x0;
  usleep(1);
  *rst = 0x1;
  usleep(1);
  *rst = 0x0;
}

void ATLASPix::setPulse(ATLASPixMatrix& matrix, uint32_t npulse, uint32_t n_up, uint32_t n_down, double voltage) {

  LOG(logDEBUG) << " Set injection voltages ";
  _hal->setBiasRegulator(INJ_1, voltage);
  _hal->powerBiasRegulator(INJ_1, true);
  _hal->setBiasRegulator(INJ_2, voltage);
  _hal->powerBiasRegulator(INJ_2, true);
  _hal->setBiasRegulator(INJ_3, voltage);
  _hal->powerBiasRegulator(INJ_3, true);
  _hal->setBiasRegulator(INJ_4, voltage);
  _hal->powerBiasRegulator(INJ_4, true);

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);

  // volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);
  volatile uint32_t* pulse_count = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x4);
  volatile uint32_t* high_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x8);
  volatile uint32_t* low_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0xC);
  volatile uint32_t* output_enable =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x10);
  // volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);

  *pulse_count = npulse;
  *high_cnt = n_up;
  *low_cnt = n_down;
  *output_enable = 0xFFFFF; // matrix.PulserMask;

  this->pulse_width = std::ceil(((npulse * n_up + npulse * n_down) * (1.0 / 160.0e6)) / 1e-6) + 10;
}

void ATLASPix::sendPulse() {

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);

  //	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE,
  // ATLASPix_READOUT_MASK);
  //
  //	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
  //	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
  // 0x4);
  //	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
  // 0x8);
  //	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
  //	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);
  //
  //
  //	 *ro = 0x20000;
  //	 *fifo_config = 0b11;

  *inj_flag = 0x1;
  usleep(this->pulse_width);
  *inj_flag = 0x0;

  //	 sleep(1);
  //	 *ro = 0x20000;
  //	 *fifo_config = 0b10;
}

void ATLASPix::resetCounters() {

  void* counter_base =
    _hal->getMappedMemoryRW(ATLASPix_COUNTER_BASE_ADDRESS, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK);

  volatile uint32_t* cnt_rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x10);
  volatile uint32_t* global_reset =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x14);

  *global_reset = 0x0;
  usleep(10);
  *global_reset = 0x1;
  usleep(10);
  *global_reset = 0x0;

  usleep(10);
  *cnt_rst = 0x0;
  usleep(1);
  *cnt_rst = 0x1;
  usleep(1);
  *cnt_rst = 0x0;
}

int ATLASPix::readCounter(int i) {
  void* counter_base =
    _hal->getMappedMemoryRW(ATLASPix_COUNTER_BASE_ADDRESS, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK);

  int value = 0;
  switch(i) {
  case 0: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x0);
    value = *cnt_value;
  } break;
  case 1: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x4);
    value = *cnt_value;
  } break;
  case 2: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x8);
    value = *cnt_value;
    break;
  }
  case 3: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0xC);
    value = *cnt_value;
    break;
  }
  default: {
    std::cout << "NON-EXISTING COUNTER, RETURN -1" << std::endl;
    value = -1;
  }
  }

  return value;
}

int ATLASPix::readCounter(ATLASPixMatrix& matrix) {
  void* counter_base =
    _hal->getMappedMemoryRW(ATLASPix_COUNTER_BASE_ADDRESS, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK);

  int value = 0;
  switch(matrix.counter) {
  case 0: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x0);
    value = *cnt_value;
  } break;
  case 1: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x4);
    value = *cnt_value;
  } break;
  case 2: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x8);
    value = *cnt_value;
    break;
  }
  case 3: {
    volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0xC);
    value = *cnt_value;
    break;
  }
  default: {
    std::cout << "NON-EXISTING COUNTER, RETURN -1" << std::endl;
    value = -1;
  }
  }

  return value;
}

void ATLASPix::pulse(uint32_t npulse, uint32_t tup, uint32_t tdown, double amplitude) {

  this->resetCounters();
  this->setPulse(theMatrix, npulse, tup, tdown, amplitude);
  // std::cout << "sending pulse" << std::endl;
  this->sendPulse();
  usleep(2000);
}

// TDAC Manipulation

void ATLASPix::MaskPixel(uint32_t col, uint32_t row) {

  theMatrix.setMask(col, row, 1);
  // std::cout << "pixel masked col:" << col << " row: " << row << " " << theMatrix.MASK[col][row] << std::endl;
  this->writeOneTDAC(theMatrix, col, row, 7);
  //this->SetPixelInjection(col, row, 1, 1, 1);
  //this->SetPixelInjection(col, row, 0, 0, 0);
}

void ATLASPix::setAllTDAC(uint32_t value) {
  this->writeUniformTDAC(theMatrix, value);
}

void ATLASPix::LoadTDAC(std::string filename) {
  theMatrix.loadTDAC(filename);
  writeAllTDAC(theMatrix);
}

void ATLASPix::writeOneTDAC(ATLASPixMatrix& matrix, uint32_t col, uint32_t row, uint32_t value) {

  matrix.setTDAC(col, row, value);

  if((matrix.flavor == ATLASPix1Flavor::M1) || (matrix.flavor == ATLASPix1Flavor::M1Iso)) {

    // Column Register

    std::string s = to_string(col);
    matrix.MatrixDACConfig->SetParameter("colinjDown" + s, 0);
    matrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
    matrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
    matrix.MatrixDACConfig->SetParameter("colinjUp" + s, 0);
    matrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
    matrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);

    if(row < 200) {
      matrix.MatrixDACConfig->SetParameter("RamDown" + s, matrix.TDAC[col][row]); // 0b1011
      // std::cout << std::bitset<4>(matrix.TDAC[col][row]) << std::endl;

      // matrix.MatrixDACConfig->SetParameter("RamUp"+s, 4, ATLASPix_Config::LSBFirst,  matrix.TDAC[col][row]); //0b1011
    } else {
      // matrix.MatrixDACConfig->SetParameter("RamDown"+s, 4, ATLASPix_Config::LSBFirst,  matrix.TDAC[col][row]); //0b1011
      matrix.MatrixDACConfig->SetParameter("RamUp" + s, matrix.TDAC[col][row]); // 0b1011
      // std::cout << std::bitset<4>(matrix.TDAC[col][row]) << std::endl;
    }

  }

  else {

    int double_col = int(std::floor(double(col) / 2));
    std::string col_s = to_string(double_col);
    if(col % 2 == 0) {
      matrix.MatrixDACConfig->SetParameter("RamL" + col_s, matrix.TDAC[col][row]);
      matrix.MatrixDACConfig->SetParameter("colinjL" + col_s, 0);
    } else {
      matrix.MatrixDACConfig->SetParameter("RamR" + col_s, matrix.TDAC[col][row]);
      matrix.MatrixDACConfig->SetParameter("colinjR" + col_s, 0);
    }
  }

  std::string row_s = to_string(row);
  matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
  matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
  matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
  matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);

  this->ProgramSR(matrix);

  matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
  matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
  matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);

  this->ProgramSR(matrix);
}

void ATLASPix::writeUniformTDAC(ATLASPixMatrix& matrix, uint32_t value) {

  std::string col_s;
  int double_col = 0;

  matrix.setUniformTDAC(value);

  // std::cout << "writing " <<  std::bitset<32>(value) << std::endl;

  if((matrix.flavor == ATLASPix1Flavor::M1) || (matrix.flavor == ATLASPix1Flavor::M1Iso)) {

    // Column Register
    for(int col = 0; col < matrix.ncol; col++) {
      std::string s = to_string(col);
      matrix.MatrixDACConfig->SetParameter("colinjDown" + s, 0);
      matrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
      matrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
      matrix.MatrixDACConfig->SetParameter("colinjUp" + s, 0);
      matrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
      matrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);

      matrix.MatrixDACConfig->SetParameter("RamDown" + s, matrix.TDAC[col][0]); // 0b1011
      matrix.MatrixDACConfig->SetParameter("RamUp" + s, matrix.TDAC[col][0]);   // 0b1011
    }
  }

  else {
    for(int col = 0; col < matrix.ncol; col++) {
      double_col = int(std::floor(double(col) / 2));
      col_s = to_string(double_col);
      if(col % 2 == 0) {
        matrix.MatrixDACConfig->SetParameter("RamL" + col_s, matrix.TDAC[col][0]);
        matrix.MatrixDACConfig->SetParameter("colinjL" + col_s, 0);
      } else {
        matrix.MatrixDACConfig->SetParameter("RamR" + col_s, matrix.TDAC[col][0]);
        matrix.MatrixDACConfig->SetParameter("colinjR" + col_s, 0);
      }
    }
  };

  for(int row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);

  for(int row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);

  for(int row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);
}

void ATLASPix::writeAllTDAC(ATLASPixMatrix& matrix) {

  std::string col_s;
  int double_col = 0;

  // std::cout << "i am here" << std::endl;

  for(int row = 0; row < matrix.nrow; row++) {
    if((matrix.flavor == ATLASPix1Flavor::M1) || (matrix.flavor == ATLASPix1Flavor::M1Iso)) {

      // Column Register
      for(int col = 0; col < matrix.ncol; col++) {
        std::string s = to_string(col);
        matrix.MatrixDACConfig->SetParameter("colinjDown" + s, 0);
        matrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
        matrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
        matrix.MatrixDACConfig->SetParameter("colinjUp" + s, 0);
        matrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
        matrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);

        if(row < 200) {
          matrix.MatrixDACConfig->SetParameter("RamDown" + s, matrix.TDAC[col][row]); // 0b1011
          matrix.MatrixDACConfig->SetParameter("RamUp" + s, matrix.TDAC[col][row]);   // 0b1011

        } else {
          matrix.MatrixDACConfig->SetParameter("RamUp" + s, matrix.TDAC[col][row]);   // 0b1011
          matrix.MatrixDACConfig->SetParameter("RamDown" + s, matrix.TDAC[col][row]); // 0b1011
        }
      }
    }

    else {
      for(int col = 0; col < matrix.ncol; col++) {
        double_col = int(std::floor(double(col) / 2));
        col_s = to_string(double_col);
        if(col % 2 == 0) {
          matrix.MatrixDACConfig->SetParameter("RamL" + col_s, matrix.TDAC[col][row]);
          matrix.MatrixDACConfig->SetParameter("colinjL" + col_s, 0);
        } else {
          matrix.MatrixDACConfig->SetParameter("RamR" + col_s, matrix.TDAC[col][row]);
          matrix.MatrixDACConfig->SetParameter("colinjR" + col_s, 0);
        }
      }
    };

    if(row % 25 == 0) {
      std::cout << "processing row : " << row << std::endl;
    }
    std::string row_s = to_string(row);

    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);

    // Toggle the line
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    this->ProgramSR(matrix);

    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    this->ProgramSR(matrix);

    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    this->ProgramSR(matrix);
  };
}

// injections

void ATLASPix::SetPixelInjection(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj_state) {
  std::string col_s;
  int double_col = 0;

  // set All injection in row and columns to off
  if(col == 999) {

    for(int colt = 0; colt < theMatrix.ncol; colt++) {
      this->SetPixelInjectionState(colt, 0, 0, 0, 0);
    }
    for(int rowt = 0; rowt < theMatrix.nrow; rowt++) {
      this->SetPixelInjectionState(0, rowt, 0, 0, 0);
      std::string row_s = to_string(rowt);
      theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    }
    this->ProgramSR(theMatrix);
    for(int rowt = 0; rowt < theMatrix.nrow; rowt++) {
      std::string row_s = to_string(rowt);
      theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    }
    this->ProgramSR(theMatrix);
  }

  if((theMatrix.flavor == ATLASPix1Flavor::M1) || (theMatrix.flavor == ATLASPix1Flavor::M1Iso)) {
    std::string s = to_string(col);

    if(row < 200) {
      theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]);   // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 3);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 3);

    } else {
      // std::cout << "up pixels" << std::endl;
      theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]);   // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 3);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 3);
    }

  } else {

    double_col = int(std::floor(double(col) / 2));
    col_s = to_string(double_col);
    if(col % 2 == 0) {
      theMatrix.MatrixDACConfig->SetParameter("RamL" + col_s, theMatrix.TDAC[col][row] & 0b111);
      theMatrix.MatrixDACConfig->SetParameter("colinjL" + col_s, inj_state);
    } else {
      theMatrix.MatrixDACConfig->SetParameter("RamR" + col_s, theMatrix.TDAC[col][row] & 0b111);
      theMatrix.MatrixDACConfig->SetParameter("colinjR" + col_s, inj_state);
    }
  }

  std::string row_s = to_string(row);
  theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
  theMatrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
  theMatrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, inj_state);
  theMatrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, ana_state);
  this->ProgramSR(theMatrix);
  theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  this->ProgramSR(theMatrix);
}

void ATLASPix::SetPixelInjectionState(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj) {
  std::string col_s;
  int double_col = 0;

  if((theMatrix.flavor == ATLASPix1Flavor::M1) || (theMatrix.flavor == ATLASPix1Flavor::M1Iso)) {
    std::string s = to_string(col);

    if(row < 200) {
      theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]);   // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 3);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 3);

    } else {
      // std::cout << "up pixels" << std::endl;
      theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]);   // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 3);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 3);
    }

  } else {

    double_col = int(std::floor(double(col) / 2));
    col_s = to_string(double_col);
    if(col % 2 == 0) {
      theMatrix.MatrixDACConfig->SetParameter("RamL" + col_s, theMatrix.TDAC[col][row] & 0b111);
      theMatrix.MatrixDACConfig->SetParameter("colinjL" + col_s, inj);
    } else {
      theMatrix.MatrixDACConfig->SetParameter("RamR" + col_s, theMatrix.TDAC[col][row] & 0b111);
      theMatrix.MatrixDACConfig->SetParameter("colinjR" + col_s, inj);
    }
  }

  std::string row_s = to_string(row);
  theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
  theMatrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
  theMatrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, inj);
  theMatrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, ana_state);
}

void ATLASPix::ResetWriteDAC() {

  for(int row = 0; row < theMatrix.nrow; row++) {
    std::string row_s = to_string(row);
    theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  }
}

void ATLASPix::SetInjectionMask(uint32_t maskx, uint32_t masky, uint32_t state) {

  for(int col = 0; col < theMatrix.ncol; col++) {
    if(((col + maskx) % theMatrix.maskx) == 0) {

      // LOG(logINFO) << "injecting in col " << col << std::endl;

      this->SetPixelInjectionState(col, 0, 0, 0, state);
    }
  }

  for(int row = 0; row < theMatrix.nrow; row++) {
    if(((row + masky) % theMatrix.masky) == 0) {
      this->SetPixelInjectionState(0, row, 0, 0, state);
      // LOG(logINFO) << "injecting in row " << row << std::endl;
    }
  };

  this->ProgramSR(theMatrix);
  this->ResetWriteDAC();
  this->ProgramSR(theMatrix);
}

// Tuning

// void ATLASPix::tune(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, bool tuning_verification) {
//  LOG(logINFO) << "Tunning " << DEVICE_NAME;
//
//  for(int TDAC_value = 0; TDAC_value < 8; TDAC_value++) {
//    matrix.setUniformTDAC(TDAC_value);
//    writeAllTDAC(matrix);
//    ComputeSCurves(matrix, vmax, nstep, npulses, 100, 100);
//    // s_curve plots
//  }
//
//  // double threshold_target = 0;
//  const int cols = matrix.ncol;
//  const int rows = matrix.nrow;
//  int TDAC_map[cols][rows]; //= {0,0};
//  // threshold_target calculation;
//  // pixel TDAC interpolation for target
//  //==> new, tuned, TDAC map
//
//  for(int col = 0; col < matrix.ncol; col++) {
//    for(int row = 0; row < matrix.nrow; row++) {
//      matrix.TDAC[col][row] = TDAC_map[col][row];
//    }
//  }
//  writeAllTDAC(matrix);
//  if(tuning_verification == true) {
//    ComputeSCurves(matrix, 0.5, 50, 128, 100, 100);
//    // S_curve plots + threshold distribution
//  }
//}

void ATLASPix::ComputeSCurves(ATLASPixMatrix& matrix, double vmax, int nstep, int npulses, int tup, int tdown) {

  std::clock_t start;
  double duration;

  start = std::clock();
  const int steps = nstep;
  const int cols = matrix.ncol;
  const int rows = matrix.nrow;
  double s_curves[steps][cols][rows]; // = {0, 0, 0};

  int step = 0;
  for(double v = 0; v <= vmax; v += (vmax / nstep)) {
    setPulse(matrix, npulses, tup, tdown, v);
    std::cout << "  bias :" << v << "V" << std::endl;

    for(int col = 0; col < matrix.ncol; col++) {
      for(int row = 0; row < matrix.nrow; row++) {
        sendPulse();
        s_curves[step][col][row] = 0;
      }
    }
    step++;
  }
  duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
  std::cout << "duration : " << duration << "s" << std::endl;
}

std::vector<pixelhit> ATLASPix::CountHits(std::vector<pixelhit> data, uint32_t maskidx, uint32_t maskidy, CounterMap& counts) {

  for(auto& hit : data) {
    if((((hit.col + maskidx) % theMatrix.maskx) == 0) && (((hit.row + maskidy) % theMatrix.masky) == 0)) {
      counts[std::make_pair(hit.col, hit.row)]++;
    }
  }
  std::vector<pixelhit> hp;
  for (auto& cnt : counts) {
	  if(cnt.second>250){
		 std::cout << "HOT PIXEL: " << cnt.first.first <<" " << cnt.first.second << std::endl;
		 this->MaskPixel(cnt.first.first,cnt.first.second);
		 pixelhit ahp;
		 ahp.col=cnt.first.first;
		 ahp.row=cnt.first.second;
		 hp.push_back(ahp);
	  }
  }

}

void ATLASPix::doSCurves(double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);

  std::vector<CounterMap> SCurveData(npoints);
  std::vector<pixelhit> hp;
  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/SCURVE_TDAC" + std::to_string(theMatrix.TDAC[0][0] >> 1) + ".txt", std::ios::out);
  // disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  for(int mx = 0; mx < theMatrix.maskx; mx++) {
    for(int my = 0; my < theMatrix.masky; my++) {

      this->SetInjectionMask(mx, my, 1);
      for(auto& pix : hp){this->MaskPixel(pix.col,pix.row);}

      vinj = vmin;
      for(int i = 0; i < npoints; i++) {
        LOG(logINFO) << "pulse height : " << vinj << std::endl;
        this->pulse(npulses, 10000, 10000, vinj);
        hp=this->CountHits(this->getDataTOvector(), mx, my, SCurveData[i]);
        vinj += dv;
      }

      for(int col = 0; col < theMatrix.ncol; col++) {
        for(int row = 0; row < theMatrix.nrow; row++) {

          if((((col + mx) % theMatrix.maskx) == 0) && (((row + my) % theMatrix.masky) == 0)) {
            disk << col << " " << row << " ";
            for(int i = 0; i < npoints; i++) {
              disk << SCurveData[i][std::make_pair(col, row)] << " ";
            }
            disk << std::endl;
          }
        }
      };

      this->SetInjectionMask(mx, my, 0);
    }
  }

  disk.close();
}

void ATLASPix::isLocked() {
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);

  if((*fifo_status >> 5) & 0b1) {
    std::cout << "yes" << std::endl;
  } else {
    std::cout << "no" << std::endl;
  }
}

uint32_t ATLASPix::getTriggerCounter() {
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* trg_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x18);
  return *trg_cnt;
}

void ATLASPix::getTriggerCount() {
  LOG(logINFO) << "Trigger accepted by FSM       " << this->getTriggerCounter() << std::endl;
  LOG(logINFO) << "Trigger accepted by FSM (ext) " << this->readCounter(2) << std::endl;
  LOG(logINFO) << "Trigger received              " << this->readCounter(3) << std::endl;
}

pearydata ATLASPix::getData() {

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

  volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
  volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
  // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
  // 0x8);
  // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
  // volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/data.txt", std::ios::out);
  disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  uint64_t fpga_ts = 0;
  uint64_t fpga_ts_last = 0;
  uint64_t fpga_ts_busy = 0;
  uint32_t timestamp = 0;
  uint32_t TrCNT = 0;

  while(true) {

    // check for stop request from another thread
    if(!this->_daqContinue.test_and_set())
      break;
    // check for new data in fifo
    if((*fifo_status & 0x1) == 0) {
      continue;
    }

    uint32_t d1 = static_cast<uint32_t>(*data);
    // std::cout << std::bitset<32>(d1) << std::endl;

    // HIT data of bit 31 is = 1
    if((d1 >> 31) == 1) {

      pixelhit hit = decodeHit(d1);
      // LOG(logINFO) << hit.col <<" " << hit.row << " " << hit.ts1 << ' ' << hit.ts2 << std::endl;
      disk << "HIT " << hit.col << "	" << hit.row << "	" << hit.ts1 << "	" << hit.ts2 << "   " << hit.tot << "	" << fpga_ts_last << "	"
           << " " << TrCNT << " " << ((timestamp >> 8) & 0xFFFF) << std::endl;
      // disk << std::bitset<32>(d1) << std::endl;
    }

    else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      // Parse the different data types (BUFFEROVERFLOW,TRIGGER,BUSY_ASSERTED)
      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        timestamp = d1 & 0xFFFFFF;
        break;
      case 0b00000001: // Buffer overflow, data after this are lost
        disk << "BUFFER_OVERFLOW" << std::endl;
        break;
      case 0b00010000: // Trigger cnt 24bits
        TrCNT = d1 & 0xFFFFFF;
        break;
      case 0b00110000: // Trigger cnt 8b + fpga_ts 16 bits
        TrCNT = TrCNT + ((d1 << 8) & 0xFF000000);
        fpga_ts = fpga_ts + (((uint64_t)d1 << 48) & 0xFFFF000000000000);
        break;
      case 0b00100000: // continuation of fpga_ts (24 bits)
        fpga_ts = fpga_ts + (((uint64_t)d1 << 24) & 0x0000FFFFFF000000);
        break;
      case 0b01100000: // End of fpga_ts (24 bits)
        fpga_ts = fpga_ts + ((d1)&0xFFFFFF);
        // LOG(logINFO) << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        disk << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        fpga_ts_last = fpga_ts;
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS

        fpga_ts_busy = d1 & 0xFFFFFF;
        disk << "BUSY_ASSERTED " << fpga_ts_busy << std::endl;

        break;

      case 0b00001000: // SERDES lock lost
        disk << "SERDES_LOCK_LOST" << std::endl;
        break;
      case 0b00001100: // SERDES lock established
        disk << "SERDES_LOCK_ESTABLISHED" << std::endl;
        break;
      case 0b00000100: // Unexpected/weird data came
        disk << "WEIRD_DATA" << std::endl;
        break;
      default: // weird stuff, should not happend
        LOG(logWARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }
  disk.close();

  // write additional information
  // std::ofstream stats(_output_directory + "/stats.txt", std::ios::out);
  // stats << "trigger_counter " << this->getTriggerCounter() << std::endl;

  pearydata dummy;
  return dummy;
}

pearydata ATLASPix::getDataTO(int maskx, int masky) {

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

  volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
  volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
  // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
  // 0x8);
  // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
  // volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/data.txt", std::ios::out);
  disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  uint64_t fpga_ts = 0;
  uint64_t fpga_ts_last = 0;
  uint64_t fpga_ts_busy = 0;
  uint32_t timestamp = 0;
  uint32_t TrCNT = 0;

  bool to = false;
  uint32_t tocnt = 0;
  uint32_t datatocnt = 0;

  while(to == false) {

    if(!this->_daqContinue.test_and_set()) {
      break;
    }

    // Check for new data in FIFO
    if((*fifo_status & 0x1) == 0) {
      // wait a microsecond and keep track of it
      usleep(1);
      tocnt += 1;

      // if timeout, leave loop
      if(tocnt == Tuning_timeout) {
        to = true;
        break;
      } else {
        continue;
      }
    }

    if(datatocnt > TuningMaxCount) {
      to = true;
      LOG(logWARNING) << "stopping data taking because of noise (over 1M hits), re-applying mask" << std::endl;
      this->ReapplyMask();
      this->reset();
      sleep(1);
      break;
    }

    uint32_t d1 = static_cast<uint32_t>(*data);

    if((d1 >> 31) == 1) {

      pixelhit hit = decodeHit(d1);
      // LOG(logINFO) << hit.col <<" " << hit.row << " " << hit.ts1 << ' ' << hit.ts2 << std::endl;
      disk << "HIT " << hit.col << "	" << hit.row << "	" << hit.ts1 << "	" << hit.ts2 << "	" << fpga_ts_last << "	"
           << " " << TrCNT << " " << ((timestamp >> 8) & 0xFFFF) << " " << (timestamp & 0xFF) << " "
           << ((fpga_ts_last >> 1) & 0xFFFF) << std::endl;
      datatocnt++;

    }

    else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        timestamp = d1 & 0xFFFFFF;
        break;
      case 0b00000001: // Buffer overflow, data after this are lost
        // disk << "BUFFER_OVERFLOW" << std::endl;
        break;
      case 0b00010000: // Trigger cnt 24bits
        TrCNT = d1 & 0xFFFFFF;
        break;
      case 0b00110000: // Trigger cnt 8b + fpga_ts 16 bits
        TrCNT = TrCNT + ((d1 << 8) & 0xFF000000);
        fpga_ts = fpga_ts + (((uint64_t)d1 << 48) & 0xFFFF000000000000);
        break;
      case 0b00100000: // continuation of fpga_ts (24 bits)
        fpga_ts = fpga_ts + (((uint64_t)d1 << 24) & 0x0000FFFFFF000000);
        break;
      case 0b01100000: // End of fpga_ts (24 bits)
        fpga_ts = fpga_ts + ((d1)&0xFFFFFF);
        // LOG(logINFO) << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        // disk << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        fpga_ts_last = fpga_ts;
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        fpga_ts_busy = d1 & 0xFFFFFF;
        // disk << "BUSY_ASSERTED " << fpga_ts_busy << std::endl;
        break;

      default: // weird stuff, should not happend
        LOG(logWARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }

  disk.close();

  LOG(logINFO) << "data count : " << datatocnt << std::endl;

  pearydata dummy;
  return dummy;
}

std::vector<pixelhit> ATLASPix::getDataTOvector() {

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

  volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
  volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
  // volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) +
  // 0x8);
  // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
  // volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

  //  make_directories(_output_directory);
  //  std::ofstream disk;
  //  disk.open(_output_directory + "/data.txt", std::ios::out);
  //  disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:   SyncedCNT:   TR_CNT:	ATPBinCounter:   ATPGreyCounter:	" << std::endl;

  uint64_t fpga_ts = 0;
  uint64_t fpga_ts_last = 0;
  uint64_t fpga_ts_busy = 0;
  uint32_t timestamp = 0;
  uint32_t TrCNT = 0;

  bool to = false;
  uint32_t tocnt = 0;
  uint32_t datatocnt = 0;
  std::vector<pixelhit> datavec;
  uint32_t datacnt = 0;

  while(to == false) {

    // check for stop request from another thread
    if(!this->_daqContinue.test_and_set())
      break;
    // check for new first half-word or restart loop

    if((*fifo_status & 0x1) == 0) {
      usleep(1);
      tocnt += 1;
      if(tocnt == Tuning_timeout) {
        to = true;
        break;
      } else {
        continue;
      }
    }

    uint32_t d1 = static_cast<uint32_t>(*data);

    if((d1 >> 31) == 1) {

      pixelhit hit = decodeHit(d1);
      datavec.push_back(hit);
      datacnt++;
    }

    else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        timestamp = d1 & 0xFFFFFF;
        break;
      case 0b00000001: // Buffer overflow, data after this are lost
        // disk << "BUFFER_OVERFLOW" << std::endl;
        break;
      case 0b00010000: // Trigger cnt 24bits
        TrCNT = d1 & 0xFFFFFF;
        break;
      case 0b00110000: // Trigger cnt 8b + fpga_ts 16 bits
        TrCNT = TrCNT + ((d1 << 8) & 0xFF000000);
        fpga_ts = fpga_ts + (((uint64_t)d1 << 48) & 0xFFFF000000000000);
        break;
      case 0b00100000: // continuation of fpga_ts (24 bits)
        fpga_ts = fpga_ts + (((uint64_t)d1 << 24) & 0x0000FFFFFF000000);
        break;
      case 0b01100000: // End of fpga_ts (24 bits)
        fpga_ts = fpga_ts + ((d1)&0xFFFFFF);
        // LOG(logINFO) << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        // disk << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        fpga_ts_last = fpga_ts;
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        fpga_ts_busy = d1 & 0xFFFFFF;
        // disk << "BUSY_ASSERTED " << fpga_ts_busy << std::endl;
        break;

      default: // weird stuff, should not happend
        LOG(logWARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }

  LOG(logINFO) << "data count : " << datacnt << std::endl;

  return datavec;
}

void ATLASPix::ReapplyMask() {

  LOG(logINFO) << "re-applying mask " << std::endl;
  for(int col = 0; col < theMatrix.ncol; col++) {
    for(int row = 0; row < theMatrix.nrow; row++) {
      if(theMatrix.MASK[col][row] == 1) {
        // LOG(logINFO) << "masking " << col << " " << row << std::endl;
        this->MaskPixel(col, row);
      }
    };
  };
}

void ATLASPix::dataTuning(double vmax, int nstep, int npulses) {

  const double margin = 0.05;

  Color::Modifier red(Color::FG_RED);
  Color::Modifier green(Color::FG_GREEN);
  Color::Modifier blue(Color::FG_BLUE);
  Color::Modifier cyan(Color::FG_CYAN);
  Color::Modifier mag(Color::FG_MAGENTA);
  Color::Modifier bold(Color::BOLD);
  Color::Modifier rev(Color::REVERSE);
  Color::Modifier def(Color::FG_DEFAULT);
  Color::Modifier reset(Color::RESET);

  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/verif.txt", std::ios::out);
  disk << "X:	Y:	   TDAC:	   COUNT:	" << std::endl;

  LOG(logINFO) << "Tuning using data for target " << vmax;
  for(int col = 0; col < theMatrix.ncol; col++) {
    for(int row = 0; row < theMatrix.nrow; row++) {

      int cur_tdac = 4;
      bool done = false;
      if(nstep == 0)
        done = true;
      uint32_t cnt = 0;
      uint32_t loop = 0;
      bool masked = false;

      this->setPulse(theMatrix, npulses, 10000, 10000, vmax);
      // this->SetPixelInjection(col,row,1,1);
      LOG(logINFO) << rev << red << "Pixel  " << col << " " << row << reset << std::endl;

      while(!done && loop < 16) {
        this->writeOneTDAC(theMatrix, col, row, cur_tdac);
        this->SetPixelInjection(col, row, 1, 1, 1);
        this->reset();
        sendPulse();
        std::vector<pixelhit> data = this->getDataTOvector();
        cnt = 0;
        for(auto hit : data) {
          if(hit.col == col && hit.row == row) {
            cnt++;
            // LOG(logINFO) << "Pixel  " << hit.col<< " " << hit.row << std::endl;
          }
        }

        LOG(logINFO) << "tdac: " << cur_tdac << " "
                     << "cnt: " << cnt << std::endl;

        if(cnt > 10 * npulses) {
          this->MaskPixel(col, row);
          masked = true;
        }

        if(cnt > (npulses * 0.5 - margin * npulses) && cnt < (npulses * 0.5 + margin * npulses)) {
          done = true;
          break;
        }

        if(cur_tdac == 7) {
          done = true;
        } else if(cur_tdac == 0) {
          done = true;
        }

        else if(cnt < (npulses * 0.5 - margin * npulses)) {
          cur_tdac -= 1;
        } else if(cnt > (npulses * 0.5 + margin * npulses)) {
          cur_tdac += 1;
        } else {
          done = true;
        }
        loop++;
      }

      if(nstep == 0) {
        this->writeOneTDAC(theMatrix, col, row, cur_tdac);
        this->SetPixelInjection(col, row, 1, 1, 1);
      }
      this->reset();
      sendPulse();
      std::vector<pixelhit> data = this->getDataTOvector();
      cnt = 0;
      // LOG(logINFO) << "Pixel  " << col << " " << row << " tdac: " << cur_tdac << " " << "cnt: " << cnt << std::endl;
      for(auto hit : data) {
        if(hit.col == col && hit.row == row) {
          cnt++;
          // LOG(logINFO) << "Pixel  " << hit.col<< " " << hit.row << std::endl;
        }
      }
      disk << col << " " << row << " " << cur_tdac << " " << cnt << std::endl;
      this->SetPixelInjection(col, row, 0, 0, 0);

      LOG(logINFO) << rev << green << " tdac: " << cur_tdac << " "
                   << "mask: " << masked << " cnt: " << cnt << reset << std::endl;
    }
  }

  disk.close();
}

// void ATLASPix::dataTuning( double vmax, int nstep, int npulses) {
//  double vstep=vmax /double(nstep-1);
//
//  LOG(logINFO) << "Tuning using data with " << DEVICE_NAME;
//  LOG(logINFO) << "VMAX= " << vmax << " vstep=" << vstep << std::endl;
//
//
//  for(int TDAC_value = 1; TDAC_value < 8; TDAC_value+=2) {
//
//
//	this->writeUniformTDAC(theMatrix,TDAC_value);
//    this->ReapplyMask();
//
//	LOG(logINFO) << "TDAC " << TDAC_value << std::endl;
//
//
//    for(unsigned int maskidx = 0; maskidx < theMatrix.maskx; ++maskidx) {
//        for(unsigned int maskidy = 0; maskidy < theMatrix.masky; ++maskidy) {
//
//        LOG(logINFO) << "setting mask id  " << maskidx << " " << maskidy << " " << (maskidx*theMatrix.masky + maskidy) <<
//        std::endl;
//    	this->SetInjectionMask(maskidx,maskidy,1);
//
//    	//for(double v = (vmax /(nstep-1)); v <= vmax; v += (vmax /(nstep-1))) {
//        for(double vi = 1; vi <=nstep; vi +=1) {
//          double v = (vi)*(vmax/(nstep));
//          //ugly hack
//          //LOG(logINFO) << "pulsing with " << v << "V" << std::endl;
//          this->setPulse(theMatrix, npulses, 10000, 10000, v);
//          std::stringstream ss;
//          ss << "PEARYDATA/gradeA06-test6/VNAC" << theMatrix.CurrentDACConfig->GetParameter("VNDACPix") << "_TDAC" <<
//          TDAC_value << "_maskid_" << (maskidx*theMatrix.masky + maskidy) << "_vpulse_" << v ;
//          this->setOutputDirectory(ss.str());
//          this->reset();
//          //this->daqStart();
//          sendPulse();
//          this->getDataTO(maskidx,maskidy);
//
//        }
//    	this->SetInjectionMask(maskidx,maskidy,0);
//      }
//    }
//    }
//
//
//
//}

void ATLASPix::VerifyTuning(double vmax, int nstep, int npulses, std::string TDACFile) {
  double vstep = vmax / double(nstep - 1);

  LOG(logINFO) << "Tuning using data with " << DEVICE_NAME;
  LOG(logINFO) << "VMAX= " << vmax << " vstep=" << vstep << std::endl;

  // 1this->LoadTDAC(TDACFile);
  this->ReapplyMask();

  for(unsigned int maskidx = 0; maskidx < theMatrix.maskx; ++maskidx) {
    for(unsigned int maskidy = 0; maskidy < theMatrix.masky; ++maskidy) {

      LOG(logINFO) << "setting mask id  " << maskidx << " " << maskidy << " " << (maskidx * theMatrix.masky + maskidy)
                   << std::endl;
      this->SetInjectionMask(maskidx, maskidy, 1);

      // for(double v = (vmax /(nstep-1)); v <= vmax; v += (vmax /(nstep-1))) {
      for(double vi = 1; vi <= nstep; vi += 1) {
        double v = (vi) * (vmax / (nstep));
        // ugly hack
        // LOG(logINFO) << "pulsing with " << v << "V" << std::endl;
        this->setPulse(theMatrix, npulses, 10000, 10000, v);
        std::stringstream ss;
        ss << "PEARYDATA/gradeA06-test7/VNAC" << theMatrix.CurrentDACConfig->GetParameter("VNDACPix") << "_TDAC" << 99
           << "_maskid_" << (maskidx * theMatrix.masky + maskidy) << "_vpulse_" << v;
        this->setOutputDirectory(ss.str());
        this->reset();
        // this->daqStart();
        sendPulse();
        this->getDataTO(maskidx, maskidy);
      }
      this->SetInjectionMask(maskidx, maskidy, 0);
    }
  }
}

// void ATLASPix::doSCurves(double vmin, double vmax, uint32_t npulses, uint32_t npoints) {
//
//  std::cout << "Ok lets get started" << std::endl;
//  int cnt = 0;
//  double vinj = vmin;
//  double dv = (vmax - vmin) / (npoints - 1);
//  std::cout << "vmin : " << vmin << " vmax : " << vmax << " dV  : " << dv << std::endl;
//
//  // Setting Time Stamp in the file name
//  std::time_t t = std::time(NULL);
//  std::tm* ptm = std::localtime(&t);
//  std::stringstream ss;
//  ss << "PEARYDATA/ATLASPixGradeA_02/"
//     << "_" << ptm->tm_year + 1900 << "_" << ptm->tm_mon + 1 << "_" << ptm->tm_mday << "@" << ptm->tm_hour + 1 << "_"
//     << ptm->tm_min + 1 << "_" << ptm->tm_sec + 1 << "_VNPix";
//
//  std::string cmd;
//  cmd += "mkdir -p ";
//  cmd += " ";
//  cmd += ss.str();
//  const int dir_err = system(cmd.c_str());
//
//  std::string filename;
//  filename += ss.str();
//  filename += "/";
//  filename += "M1_VNDAC_";
//  filename += std::to_string(theMatrix.CurrentDACConfig->GetParameter("VNDACPix"));
//  filename += "_TDAC_";
//  filename += std::to_string(theMatrix.TDAC[0][0] >> 1);
//  // filename+=ss.str();
//  filename += ".txt";
//
//  std::cout << "writing to file : " << filename << std::endl;
//
//  std::ofstream myfile;
//  myfile.open(filename);
//
//  myfile << npoints << std::endl;
//
//  std::clock_t start;
//  double duration;
//
//  for(int col = 0; col < theMatrix.ncol; col++) {
//    for(int row = 0; row < theMatrix.nrow; row++) {
//
//      // start = std::clock();
//
//      if(row % 5 == 0) {
//        std::cout << "X: " << col << " Y: " << row << "\n";
//      }
//
//      vinj = vmin;
//      // this->SetPixelInjection(theMatrix,0,0,1,1);
//      // this->SetPixelInjection(theMatrix,0,0,0,0);
//
//      this->SetPixelInjection(col, row, 1, 1,1);
//      this->resetCounters();
//
//      for(int i = 0; i < npoints; i++) {
//        this->pulse(npulses, 1000, 1000, vinj);
//        // cnt=this->readCounter(theMatrixISO);
//        cnt = this->readCounter(theMatrix);
//        // this->getData();
//        myfile << vinj << " " << cnt << " ";
//        // std::cout << "V : " << vinj << " count : " << cnt << std::endl;
//        vinj += dv;
//      }
//
//      this->SetPixelInjection(col, row, 0, 0,0);
//      myfile << std::endl;
//
//      // duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//      // std::cout << duration << " s \n" ;
//    }
//  }
//
//  myfile.close();
//}

void ATLASPix::doSCurves(std::string basefolder, double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  std::cout << "Ok lets get started" << std::endl;
  int cnt = 0;
  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);
  std::cout << "vmin : " << vmin << " vmax : " << vmax << " dV  : " << dv << std::endl;

  std::string filename;
  filename += basefolder;
  filename += "/";
  filename += "M1_VNDAC_";
  filename += std::to_string(theMatrix.CurrentDACConfig->GetParameter("VNDACPix"));
  filename += "_TDAC_";
  filename += std::to_string(theMatrix.TDAC[0][0] >> 1);
  // filename+=ss.str();
  filename += ".txt";

  std::cout << "writing to file : " << filename << std::endl;

  std::ofstream myfile;
  myfile.open(filename);

  myfile << npoints << std::endl;

  std::clock_t start;
  double duration;

  for(int col = 0; col < theMatrix.ncol; col++) {
    for(int row = 0; row < theMatrix.nrow; row++) {

      // start = std::clock();

      if(row % 5 == 0) {
        std::cout << "X: " << col << " Y: " << row << "\n";
      }

      vinj = vmin;
      // this->SetPixelInjection(theMatrix,0,0,1,1);
      // this->SetPixelInjection(theMatrix,0,0,0,0);

      this->SetPixelInjection(col, row, 1, 1, 1);
      this->resetCounters();

      for(int i = 0; i < npoints; i++) {
        this->pulse(npulses, 1000, 1000, vinj);
        // cnt=this->readCounter(theMatrixISO);
        cnt = this->readCounter(theMatrix);
        // this->getData();
        myfile << vinj << " " << cnt << " ";
        // std::cout << "V : " << vinj << " count : " << cnt << std::endl;
        vinj += dv;
      }

      this->SetPixelInjection(col, row, 0, 0, 0);
      myfile << std::endl;

      // duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
      // std::cout << duration << " s \n" ;
    }
  }

  myfile.close();
}

void ATLASPix::TDACScan(
  std::string basefolder, int VNDAC, int step, double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  this->WriteConfig(basefolder + "/config");

  theMatrix.CurrentDACConfig->SetParameter("VNDACPix", VNDAC);

  for(int tdac = 0; tdac <= 7; tdac += step) {

    this->setAllTDAC(tdac);
    this->doSCurves(basefolder, vmin, vmax, npulses, npoints);
  }
}

// CaR Board related

void ATLASPix::reset() {
  // LOG(logINFO) << "Resetting " << DEVICE_NAME;

  double thor = theMatrix.ThPix;

  this->setThreshold(1.8);

  // RO register reset
  theMatrix.CurrentDACConfig->SetParameter("RO_res_n", 0);
  theMatrix.CurrentDACConfig->SetParameter("Ser_res_n", 0);
  theMatrix.CurrentDACConfig->SetParameter("Aur_res_n", 0);
  theMatrix.CurrentDACConfig->SetParameter("Reset", 1);

  // Analog reg reset
  int VNPixor = theMatrix.CurrentDACConfig->GetParameter("VNPix");
  int VNCompPixor = theMatrix.CurrentDACConfig->GetParameter("VNcompPix");
  theMatrix.CurrentDACConfig->SetParameter("VNPix", 0);
  theMatrix.CurrentDACConfig->SetParameter("VNcompPix", 0);

  this->ProgramSR(theMatrix);

  usleep(1000);

  // RO register reset
  theMatrix.CurrentDACConfig->SetParameter("RO_res_n", 1);
  theMatrix.CurrentDACConfig->SetParameter("Ser_res_n", 1);
  theMatrix.CurrentDACConfig->SetParameter("Aur_res_n", 1);
  theMatrix.CurrentDACConfig->SetParameter("Reset", 0);

  // Analog reg reset
  theMatrix.CurrentDACConfig->SetParameter("VNPix", VNPixor);
  theMatrix.CurrentDACConfig->SetParameter("VNcompPix", VNCompPixor);

  this->ProgramSR(theMatrix);

  this->setThreshold(thor);

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b10000;
  usleep(50);
  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b00000;
}

std::string ATLASPix::getName() {
  return DEVICE_NAME;
}

void ATLASPix::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up ATLASPix";
  std::cout << '\n';

  this->setVoltage("VDDD", ATLASPix_VDDD, ATLASPix_VDDD_CURRENT);
  this->switchOn("VDDD");

  this->setVoltage("VDDA", ATLASPix_VDDA, ATLASPix_VDDA_CURRENT);
  this->switchOn("VDDA");

  this->setVoltage("VSSA", ATLASPix_VSSA, ATLASPix_VSSA_CURRENT);
  this->switchOn("VSSA");

  // Analog biases

  this->setVoltage("GNDDACPix", theMatrix.GNDDACPix);
  this->switchOn("GNDDACPix");

  this->setVoltage("VMinusPix", theMatrix.VMINUSPix);
  this->switchOn("VMinusPix");

  this->setVoltage("GatePix", theMatrix.GatePix);
  this->switchOn("GatePix");

  // Threshold and Baseline

  this->setVoltage("ThPix", theMatrix.ThPix);
  this->switchOn("ThPix");

  this->setVoltage("BLPix", theMatrix.BLPix);
  this->switchOn("BLPix");
}

void ATLASPix::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off ATLASPix";

  LOG(logDEBUG) << "Powering off VDDA";
  this->switchOff("VDDA");

  LOG(logDEBUG) << "Powering off VDDD";
  this->switchOff("VDDD");

  LOG(logDEBUG) << "Powering off VSSA";
  this->switchOff("VSSA");

  LOG(logDEBUG) << "Turning off GNDDacPix";
  this->switchOff("GNDDACPix");

  LOG(logDEBUG) << "Turning off VMinusPix_M1";
  this->switchOff("VMinusPix");

  LOG(logDEBUG) << "Turning off GatePix_M1";
  this->switchOff("GatePix");
}

// daq thread implementation
namespace {
  using TimeoutClock = std::chrono::steady_clock;
  using TimeoutDuration = std::chrono::steady_clock::duration;
  using TimeoutTimepoint = std::chrono::steady_clock::time_point;
} // unnamed namespace

void ATLASPix::daqStart() {
  // ensure only one daq thread is running
	this->reset();
	if(_daqThread.joinable()) {
    LOG(logWARNING) << "Data aquisition is already running";
    return;
  }
  // arm the stop flag and start running
  this->resetCounters();
  _daqContinue.test_and_set();
  _daqThread = std::thread(&ATLASPix::runDaq, this);
  // LOG(logINFO) << "acquisition started" << std::endl;
}

void ATLASPix::daqStop() {
  // signal to daq thread that we want to stop and wait until it does
  _daqContinue.clear();
  _daqThread.join();
  // LOG(logINFO) << "Trigger count at end of run : " << this->getTriggerCounter() << std::endl;
}

void ATLASPix::runDaq() {
  getData();
}

void ATLASPix::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_4) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_4) << "A";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";

  LOG(logINFO) << "VSSA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_2) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_2) << "A";
}

void ATLASPix::WriteConfig(std::string name) {
  make_directories(_output_directory);
  theMatrix.writeGlobal(_output_directory + "/" + name + ".cfg");
  theMatrix.writeTDAC(_output_directory + "/" + name + "_TDAC.cfg");
}

void ATLASPix::LoadConfig(std::string basename) {
  theMatrix.loadGlobal(basename + ".cfg");
  this->ProgramSR(theMatrix);
  // 2018-02-14 msmk:
  // not sure if this is correct, but the previous version did a manual
  // power up here as well. Could this be replaced by a call
  // to powerUp directly? Is this the intended functionality, i.e.
  // this loads configuration data from file and powers everything up or
  // should this be just the loading which must be followed up by the actual
  // powerUp command?
  this->setVoltage("GNDDACPix", theMatrix.GNDDACPix);
  this->switchOn("GNDDACPix");
  this->setVoltage("VMinusPix", theMatrix.VMINUSPix);
  this->switchOn("VMinusPix");
  this->setVoltage("GatePix", theMatrix.GatePix);
  this->switchOn("GatePix");
  this->setVoltage("BLPix", theMatrix.BLPix);
  this->switchOn("BLPix");
  this->setVoltage("ThPix", theMatrix.ThPix);
  this->switchOn("ThPix");
  this->LoadTDAC(basename + "_TDAC.cfg");
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  ATLASPix* mDevice = new ATLASPix(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
