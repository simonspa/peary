/**
 * Caribou implementation for the ATLASPix
 */

#include "ATLASPixDevice.hpp"

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

uint32_t reverseBits(uint8_t n, uint32_t length = 8) {
  uint32_t x = 0;
  for(auto i = length - 1; n;) {
    x |= (n & 1) << i;
    n >>= 1;
    --i;
  }
  return x;
}

// BASIC Configuration

uint32_t grey_decode(uint32_t g, uint32_t length) {
  for(uint32_t bit = 1U << (length - 1); bit > 1; bit >>= 1) {
    if(g & bit)
      g ^= bit >> 1;
  }
  return g;
}

pixelhit decodeHit(uint32_t hit, uint32_t ckdivend2 = 1, bool gray_decoding_state = false) {
  pixelhit tmp;

  tmp.col = (hit >> 25) & 0b11111;

  tmp.row = (hit >> 16) & 0x1FF;
  tmp.ts1 = (hit >> 6) & 0x3FF;
  tmp.ts2 = hit & 0x3F;

  if(gray_decoding_state == false) {
    tmp.ts1 = grey_decode((tmp.ts1), 10);
    tmp.ts2 = grey_decode((tmp.ts2), 6);
  }

  uint32_t divider = ckdivend2 + 1;
  uint32_t shift = 0;
  // TOT decoding
  if(((divider & (divider - 1)) == 0))
    shift = int(log2(divider));
  else
    LOG(WARNING) << ": ckdivend2 yield an non power of 2 clock divider, please don't do that, TOT might be rubbish"
                 << std::endl;
  ;

  uint32_t ts1_scaled_to_Tts2 = (tmp.ts1 << 1);
  ts1_scaled_to_Tts2 = ts1_scaled_to_Tts2 >> shift;
  ts1_scaled_to_Tts2 = ts1_scaled_to_Tts2 & 0x3F;
  if(ts1_scaled_to_Tts2 < tmp.ts2) {
    tmp.tot = tmp.ts2 - ts1_scaled_to_Tts2;
  } else // rollover
  {
    tmp.tot = 64 - ts1_scaled_to_Tts2 + tmp.ts2;
  }

  tmp.tot = tmp.tot & 0x3F;

  return tmp;
}

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
} // namespace Color

ATLASPixDevice::ATLASPixDevice(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), ATLASPix_DEFAULT_I2C), _daqContinue(ATOMIC_FLAG_INIT),
      _output_directory("PEARYDATA") {

  // Register custom commands with the dispatcher:
  _dispatcher.add("setOutputDirectory", &ATLASPixDevice::setOutputDirectory, this);
  _dispatcher.add("dataTuning", &ATLASPixDevice::dataTuning, this);
  _dispatcher.add("VerifyTuning", &ATLASPixDevice::VerifyTuning, this);
  _dispatcher.add("lock", &ATLASPixDevice::lock, this);
  _dispatcher.add("unlock", &ATLASPixDevice::unlock, this);
  _dispatcher.add("setThreshold", &ATLASPixDevice::setThreshold, this);
  _dispatcher.add("setVMinus", &ATLASPixDevice::setVMinus, this);
  _dispatcher.add("getTriggerCount", &ATLASPixDevice::getTriggerCount, this);
  _dispatcher.add("pulse", &ATLASPixDevice::pulse, this);
  _dispatcher.add("SetPixelInjection", &ATLASPixDevice::SetPixelInjection, this);
  //_dispatcher.add("doSCurve", &ATLASPixDevice::doSCurve, this);
  _dispatcher.add("doSCurves", &ATLASPixDevice::doSCurves, this);
  _dispatcher.add("doSCurvesAndWrite", &ATLASPixDevice::doSCurvesAndWrite, this);
  _dispatcher.add("setAllTDAC", &ATLASPixDevice::setAllTDAC, this);
  //_dispatcher.add("doNoiseCurve", &ATLASPixDevice::doNoiseCurve, this);
  _dispatcher.add("LoadTDAC", &ATLASPixDevice::LoadTDAC, this);
  _dispatcher.add("LoadConfig", &ATLASPixDevice::LoadConfig, this);
  _dispatcher.add("WriteConfig", &ATLASPixDevice::WriteConfig, this);
  _dispatcher.add("TDACScan", &ATLASPixDevice::TDACScan, this);
  _dispatcher.add("SetMatrix", &ATLASPixDevice::SetMatrix, this);
  _dispatcher.add("MaskPixel", &ATLASPixDevice::MaskPixel, this);
  _dispatcher.add("isLocked", &ATLASPixDevice::isLocked, this);
  _dispatcher.add("powerStatusLog", &ATLASPixDevice::powerStatusLog, this);
  _dispatcher.add("StopMonitorPower", &ATLASPixDevice::StopMonitorPower, this);
  _dispatcher.add("MonitorPower", &ATLASPixDevice::MonitorPower, this);
  _dispatcher.add("SetInjectionOff", &ATLASPixDevice::SetInjectionOff, this);
  _dispatcher.add("ReapplyMask", &ATLASPixDevice::ReapplyMask, this);
  _dispatcher.add("NoiseRun", &ATLASPixDevice::NoiseRun, this);
  _dispatcher.add("MaskColumn", &ATLASPixDevice::MaskColumn, this);
  _dispatcher.add("setOutput", &ATLASPixDevice::setOutput, this);
  _dispatcher.add("FindHotPixels", &ATLASPixDevice::FindHotPixels, this);
  _dispatcher.add("MeasureTOT", &ATLASPixDevice::MeasureTOT, this);
  _dispatcher.add("doSCurvesPixel", &ATLASPixDevice::doSCurvePixel, this);
  _dispatcher.add("resetFIFO", &ATLASPixDevice::resetFIFO, this);
  _dispatcher.add("PulseTune", &ATLASPixDevice::PulseTune, this);

  // Configuring the clock
  LOG(INFO) << "Setting clock circuit on CaR board " << DEVICE_NAME;
  configureClock();

  _registers.add(ATLASPix_REGISTERS);

  // Always set up common periphery for all matrices:
  _periphery.add("VDDD", PWR_OUT_4);
  _periphery.add("VDDA", PWR_OUT_3);
  _periphery.add("VSSA", PWR_OUT_2);
  _periphery.add("VCC25", PWR_OUT_5);
  _periphery.add("VDDRam", PWR_OUT_1);
  _periphery.add("VDDHigh", PWR_OUT_6);

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

  data_type = "binary";

  // Check for matrix value in the configuration:
  if(_config.Has("matrix")) {
    SetMatrix(_config.Get<std::string>("matrix"));

    // Configure default voltages:
    theMatrix.VMINUSPix = _config.Get("vminuspix", theMatrix.VMINUSPix);
    theMatrix.GNDDACPix = _config.Get("GNDDACPix", theMatrix.GNDDACPix);
    theMatrix.GatePix = _config.Get("GatePix", theMatrix.GatePix);
    theMatrix.VNFBPix = _config.Get("VNFBPix", theMatrix.VNFBPix);
    theMatrix.BLResPix = _config.Get("BLResPix", theMatrix.BLResPix);
    theMatrix.VMain2 = _config.Get("VMain2", theMatrix.VMain2);
    theMatrix.ThPix = _config.Get("VThPix", theMatrix.ThPix);
    theMatrix.BLPix = _config.Get("VBLPix", theMatrix.BLPix);
  }

  // Define output:
  setOutput(_config.Get("output", "binary"));
  setOutputDirectory(_config.Get("output_directory", "."));
}

ATLASPixDevice::~ATLASPixDevice() {
  LOG(INFO) << DEVICE_NAME << ": Shutdown, delete device.";
  daqStop(); // does nothing if no daq thread is running
  powerOff();
}

void ATLASPixDevice::setOutputDirectory(std::string dir) {
  LOG(INFO) << "Setting output directory to: " << dir;
  _output_directory = std::move(dir);
}

void ATLASPixDevice::SetMatrix(std::string matrix) {

  // avoid stupid case mistakes
  std::string name = matrix;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if(name == "m1") {

    _periphery.add("GNDDACPix", BIAS_9);
    _periphery.add("VMinusPix", BIAS_5);
    _periphery.add("GatePix", BIAS_2);
    _periphery.add("VThPix", BIAS_25);
    _periphery.add("VBLPix", BIAS_17);
    _periphery.add("VMinusPD", BIAS_7);
    _periphery.add("VNFBPix", BIAS_11);
    _periphery.add("BLResPix", BIAS_10);
    _periphery.add("VMain2", BIAS_24);

    theMatrix.initializeM1();

  } else if(name == "m1iso") {

    _periphery.add("GNDDACPix", BIAS_12);
    _periphery.add("VMinusPix", BIAS_8);
    _periphery.add("GatePix", BIAS_3);
    _periphery.add("VThPix", BIAS_31);
    _periphery.add("VBLPix", BIAS_20);

    theMatrix.initializeM1Iso();

  } else if(name == "m2") {

    _periphery.add("GNDDACPix", BIAS_6);
    _periphery.add("VMinusPix", BIAS_4);
    _periphery.add("GatePix", BIAS_1);
    _periphery.add("VThPix", BIAS_28);
    _periphery.add("VBLPix", BIAS_23);

    theMatrix.initializeM2();

  } else {
    LOG(ERROR) << "Unknown matrix flavor '" << matrix << "'";
  }
}

void ATLASPixDevice::SetScanningMask(uint32_t mx, uint32_t my) {

  theMatrix.maskx = mx;
  theMatrix.masky = my;
}

void ATLASPixDevice::setOutput(std::string datatype) {

  if(datatype == "binary") {
    data_type = "binary";
  } else if(datatype == "text") {
    data_type = "text";
  } else if(datatype == "raw") {
    data_type = "raw";
  } else {
    LOG(INFO) << "Data type not recongnized, using binary ";
    data_type = "binary";
  }
}

void ATLASPixDevice::configure() {

  LOG(INFO) << "Configuring " << DEVICE_NAME << " with default configuration";

  this->resetPulser();
  this->resetCounters();

  // this->powerOn();
  usleep(1000);

  // Build the SR string with default values and shift in the values in the chip
  this->ProgramSR(theMatrix);

  // this->ComputeSCurves(0,0.5,50,128,100,100);
  std::cout << "sending default TDACs " << std::endl;

  this->writeUniformTDAC(theMatrix, 0b0000);
  this->setSpecialRegister("ro_enable", 0);
  this->setSpecialRegister("armduration", 2000);

  //  for(int col = 0; col < theMatrix.ncol; col++) {
  //    for(int row = 0; row < theMatrix.nrow; row++) {
  //      this->SetPixelInjectionState(col, row, 0, 0, 1);
  //    }
  //  }
  //
  //  this->ProgramSR(theMatrix);
  //  this->ResetWriteDAC();
  //  this->ProgramSR(theMatrix);
  //
  //  for(int col = 0; col < theMatrix.ncol; col++) {
  //    for(int row = 0; row < theMatrix.nrow; row++) {
  //      this->SetPixelInjectionState(col, row, 0, 0, 0);
  //    }
  //  }

  this->ProgramSR(theMatrix);
  this->ResetWriteDAC();
  this->ProgramSR(theMatrix);

  // Print power status information:
  powerStatusLog();

  // locking and unlocking seems to be good practice: "it works better afterwards" I have been told:
  lock();
  unlock();

  // Set all TDAC:
  this->setAllTDAC(0);

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();

  // Check lock:
  if(_hal->isLockedSI5345()) {
    LOG(INFO) << "PLL locked to external clock...";
  } else {
    LOG(WARNING) << "Cannot lock to external clock, PLL will continue in freerunning mode...";
  }
}

void ATLASPixDevice::lock() {

  theMatrix.CurrentDACConfig->SetParameter("unlock", 0x0);
  this->ProgramSR(theMatrix);

  // usleep(10000);

  // this->ProgramSR(theMatrix);

  // this->ProgramSR(theMatrix);
}

void ATLASPixDevice::unlock() {

  theMatrix.CurrentDACConfig->SetParameter("unlock", 0b1010);
  this->ProgramSR(theMatrix);
}

void ATLASPixDevice::setThreshold(double threshold) {

  theMatrix.VoltageDACConfig->SetParameter("ThPix", static_cast<int>(floor(255 * threshold / 1.8)));

  this->ProgramSR(theMatrix);

  LOG(DEBUG) << " VThPix ";
  this->setVoltage("VThPix", threshold);
  this->switchOn("VThPix");
  theMatrix.ThPix = threshold;
}

void ATLASPixDevice::setVMinus(double vminus) {

  theMatrix.VMINUSPix = vminus;
  LOG(DEBUG) << " VMinusPix ";
  this->setVoltage("VMinusPix", vminus);
  this->switchOn("VMinusPix");
}

template <typename T> uint32_t ATLASPixDevice::getSpecialRegister(std::string name) {
  throw RegisterInvalid("Unknown readable register with \"special\" flag: " + name);
}

void ATLASPixDevice::setSpecialRegister(std::string name, uint32_t value) {

  // atlaspix registers
  if(std::find(std::begin(AXI_registers), std::end(AXI_registers), name) == std::end(AXI_registers)) {

    theMatrix.CurrentDACConfig->SetParameter(name, value);
    this->ProgramSR(theMatrix);

  }

  // axi registers
  else {

    if(name == "ro_enable") {

      this->ro_enable = value;

      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
      // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base)
      // + 0x4);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      // volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
      // volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

      *fifo_config = (*fifo_config & 0xFFFFFFFE) + ((value)&0b1);
      //    if(value == 1) {
      //      this->resetCounters();
      //    }

    } else if(name == "trigger_enable") {

      trigger_enable = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

      *fifo_config = (*fifo_config & 0xFFFD) + ((value << 1) & 0b10);

    } else if(name == "edge_sel") {

      edge_sel = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      *fifo_config = (*fifo_config & 0xFFFFFFFB) + ((value << 2) & 0b0100);

    }

    else if(name == "busy_when_armed") {

      busy_when_armed = value;

      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      *fifo_config = (*fifo_config & 0xFFFFFFF7) + ((value << 3) & 0b1000);
    }

    else if(name == "armduration") {

      armduration = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
      // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base)
      // + 0x4); volatile uint32_t* fifo_config = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8); volatile uint32_t* leds = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
      volatile uint32_t* config2 = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);

      *config2 = ((value)&0xFFFFFF);

    } else if(name == "trigger_always_armed") {

      trigger_always_armed = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
      // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base)
      // + 0x4); volatile uint32_t* fifo_config = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8); volatile uint32_t* leds = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

      *fifo_config = (*fifo_config & 0xFFFFFFBF) + ((value << 6) & 0b1000000);

    }

    else if(name == "t0_enable") {
      t0_enable = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

      // volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
      // volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base)
      // + 0x4); volatile uint32_t* fifo_config = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8); volatile uint32_t* leds = reinterpret_cast<volatile
      // uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

      *fifo_config = (*fifo_config & 0xFFFFFF7F) + ((value << 7) & 0b10000000);

    } else if(name == "trigger_injection") {
      trigger_injection = value;
      void* pulser_base =
        _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
      volatile uint32_t* triggerinj =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x18);
      *triggerinj = value;

    } else if(name == "gray_decode") {

      gray_decode = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      *fifo_config = (*fifo_config & 0xFBFF) + ((value << 10) & 0b010000000000);
      gray_decoding_state = value;

    }
    /*
      else if(name == "ext_clk") {

            void* readout_base =
              _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
            volatile uint32_t* fifo_config =
              reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
            *fifo_config = (*fifo_config & 0xFDFF) + ((value << 9) & 0b001000000000);
          }
    */
    else if(name == "send_fpga_ts") {
      send_fpga_ts = value;
      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      *fifo_config = (*fifo_config & 0xF7FF) + ((value << 11) & 0b100000000000);
    } else if(name == "filter_hp") {
      filter_hp = value;
    } else if(name == "filter_weird_data") {
      filter_weird_data = value;
    } else if(name == "hw_masking") {
      HW_masking = value;
    } else if(name == "t0_out_periodic") {

      void* readout_base =
        _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
      volatile uint32_t* fifo_config =
        reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
      *fifo_config = (*fifo_config & 0xEFFF) + ((value << 12) & 0b1000000000000);
    } else if(name == "blpix") {
      double dval = static_cast<double>(value) / 1000;
      theMatrix.BLPix = dval;
      this->setVoltage("VBLPix", dval);
      this->switchOn("VThPix");
      LOG(DEBUG) << "set blpix to " << theMatrix.BLPix;
    } else if(name == "thpix") {
      double dval = static_cast<double>(value) / 1000;
      theMatrix.VoltageDACConfig->SetParameter("ThPix", static_cast<unsigned int>(floor(255 * dval / 1.8)));
      this->ProgramSR(theMatrix);
      theMatrix.ThPix = dval;
      this->setVoltage("VThPix", dval);
      this->switchOn("VThPix");
      LOG(DEBUG) << "set thpix to " << theMatrix.ThPix;
    } else {
      throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
    }
  }
}

void ATLASPixDevice::configureClock() {
  /*
    // Check of we should configure for external or internal clock, default to external:
    if(_config.Get<bool>("clock_internal", false)) {
      LOG(DEBUG) << DEVICE_NAME << ": Configure internal clock source, free running, not locking";
      _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers_free, SI5345_REVB_REG_CONFIG_NUM_REGS_FREE);
      mDelay(100); // let the PLL lock
    } else {
  */
  LOG(DEBUG) << DEVICE_NAME << ": Configure external clock source, locked to TLU input clock";
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
  LOG(DEBUG) << "Waiting for clock to lock...";

  // Try for a limited time to lock, otherwise abort:
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  while(!_hal->isLockedSI5345()) {
    auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
    if(dur.count() > 0.5)
      // throw DeviceException("Cannot lock to external clock.");
      break;
  }
  if(_hal->isLockedSI5345()) {
    LOG(INFO) << "PLL locked to external clock...";
  } else {
    LOG(INFO) << "Cannot lock to external clock, PLL will continue in freerunning mode...";
  }
  //  }
}

void ATLASPixDevice::ProgramSR(const ATLASPixMatrix& matrix) {

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

  usleep(100);
  *output_enable = matrix.SRmask;
  usleep(100);
  *Config_flag = 0x1;
  //  usleep(300000);
  //  *Config_flag = 0;
  //  *output_enable = 0x0;
  //  usleep(100);

  // Sync RO state machine ckdivend with the one in the chip
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);
  *ro = (*ro & 0xFFFFFF00) + (matrix.CurrentDACConfig->GetParameter("ckdivend") & 0xFF);
}

// Injection and pulser

void ATLASPixDevice::resetPulser() {

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);
  usleep(1);
  *rst = 0x0;
  usleep(1);
  *rst = 0x1;
  usleep(1);
  *rst = 0x0;
}

void ATLASPixDevice::setPulse(
  ATLASPixMatrix& /* matrix */, uint32_t npulse, uint32_t n_up, uint32_t n_down, double voltage) {

  LOG(DEBUG) << " Set injection voltages ";
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

void ATLASPixDevice::sendPulse() {

  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);

  *inj_flag = 0x1;
  usleep(this->pulse_width);
  *inj_flag = 0x0;
}

void ATLASPixDevice::resetCounters() {

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

int ATLASPixDevice::readCounter(int i) {
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

int ATLASPixDevice::readCounter(ATLASPixMatrix& matrix) {
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

void ATLASPixDevice::pulse(uint32_t npulse, uint32_t tup, uint32_t tdown, double amplitude) {

  this->resetCounters();
  this->setPulse(theMatrix, npulse, tup, tdown, amplitude);
  // std::cout << "sending pulse" << std::endl;
  this->sendPulse();
  usleep(2000);
}

// TDAC Manipulation

void ATLASPixDevice::FindHotPixels(uint32_t threshold) {

  CounterMap counts;
  std::vector<pixelhit> data = this->getDataTimer(1000);

  for(auto& hit : data) {
    counts[std::make_pair(hit.col, hit.row)]++;
  }

  for(auto& cnt : counts) {
    if(cnt.second > threshold) {
      std::cout << "MaskPixel  " << cnt.first.first << " " << cnt.first.second << " 0" << std::endl;
      this->MaskPixel(cnt.first.first, cnt.first.second);
    }
  }
}

void ATLASPixDevice::MaskPixel(uint32_t col, uint32_t row) {

  theMatrix.setMask(col, row, 1);
  // std::cout << "pixel masked col:" << col << " row: " << row << " " << theMatrix.MASK[col][row] << std::endl;

  if(HW_masking) {
    this->writeOneTDAC(theMatrix, col, row, 7);
  }

  pixelhit pix;
  pix.col = col;
  pix.row = row;
  if(filter_hp) {
    if(std::find(hplist.begin(), hplist.end(), pix) == hplist.end()) {
      hplist.push_back(pix);
    }
  }

  // this->SetPixelInjection(col, row, 1, 1, 1);
  // this->SetPixelInjection(col, row, 0, 0, 0);
}

void ATLASPixDevice::MaskColumn(uint32_t col) {

  for(uint32_t row = 0; row < theMatrix.nrow; row++) {

    theMatrix.setTDAC(col, row, 7);

    if((theMatrix.flavor == ATLASPix1Flavor::M1) || (theMatrix.flavor == ATLASPix1Flavor::M1Iso)) {

      // Column Register

      std::string s = to_string(col);
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);

      if(row < 200) {
        theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
        // std::cout << std::bitset<4>(theMatrix.TDAC[col][row]) << std::endl;

        // theMatrix.MatrixDACConfig->SetParameter("RamUp"+s, 4, ATLASPix_Config::LSBFirst,  theMatrix.TDAC[col][row]);
        // //0b1011
      } else {
        // theMatrix.MatrixDACConfig->SetParameter("RamDown"+s, 4, ATLASPix_Config::LSBFirst,  theMatrix.TDAC[col][row]);
        // //0b1011
        theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]); // 0b1011
        // std::cout << std::bitset<4>(theMatrix.TDAC[col][row]) << std::endl;
      }

    }

    else {

      int double_col = int(std::floor(double(col) / 2));
      std::string col_s = to_string(double_col);
      if(col % 2 == 0) {
        theMatrix.MatrixDACConfig->SetParameter("RamL" + col_s, theMatrix.TDAC[col][row]);
        theMatrix.MatrixDACConfig->SetParameter("colinjL" + col_s, 0);
      } else {
        theMatrix.MatrixDACConfig->SetParameter("RamR" + col_s, theMatrix.TDAC[col][row]);
        theMatrix.MatrixDACConfig->SetParameter("colinjR" + col_s, 0);
      }
    }

    std::string row_s = to_string(row);
    theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    theMatrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    theMatrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    theMatrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  }

  this->ProgramSR(theMatrix);

  for(uint32_t row = 0; row < theMatrix.nrow; row++) {
    std::string row_s = to_string(row);

    theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    theMatrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    theMatrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    theMatrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  }

  this->ProgramSR(theMatrix);
}

void ATLASPixDevice::setAllTDAC(uint32_t value) {
  this->writeUniformTDAC(theMatrix, value);
  this->ReapplyMask();
}

void ATLASPixDevice::LoadTDAC(std::string filename) {
  theMatrix.loadTDAC(filename);
  writeAllTDAC(theMatrix);
}

void ATLASPixDevice::writeOneTDAC(ATLASPixMatrix& matrix, uint32_t col, uint32_t row, uint32_t value) {

  matrix.setTDAC(col, row, value);

  for(uint32_t col_i = 0; col_i < theMatrix.ncol; col_i++) {
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
  }

  for(uint32_t row_i = 0; row_i < theMatrix.nrow; row_i++) {
    std::string row_s = to_string(row_i);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  }
  this->ProgramSR(matrix);

  std::string row_s = to_string(row);
  matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
  this->ProgramSR(matrix);
  matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
}

void ATLASPixDevice::writeUniformTDAC(ATLASPixMatrix& matrix, uint32_t value) {

  std::string col_s;
  int double_col = 0;

  matrix.setUniformTDAC(value);

  // std::cout << "writing " <<  std::bitset<32>(value) << std::endl;

  if((matrix.flavor == ATLASPix1Flavor::M1) || (matrix.flavor == ATLASPix1Flavor::M1Iso)) {

    // Column Register
    for(uint32_t col = 0; col < matrix.ncol; col++) {
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
    for(uint32_t col = 0; col < matrix.ncol; col++) {
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

  for(uint32_t row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);

  for(uint32_t row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);

  for(uint32_t row = 0; row < matrix.nrow; row++) {

    // std::cout << "processing row : " << row << std::endl;
    std::string row_s = to_string(row);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, 0);
    matrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, 0);
  };

  this->ProgramSR(matrix);
}

void ATLASPixDevice::writeAllTDAC(ATLASPixMatrix& matrix) {

  std::string col_s;
  int double_col = 0;

  // std::cout << "i am here" << std::endl;

  for(uint32_t row = 0; row < matrix.nrow; row++) {
    if((matrix.flavor == ATLASPix1Flavor::M1) || (matrix.flavor == ATLASPix1Flavor::M1Iso)) {

      // Column Register
      for(uint32_t col = 0; col < matrix.ncol; col++) {
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
      for(uint32_t col = 0; col < matrix.ncol; col++) {
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

    for(uint32_t arow = 0; arow < matrix.nrow; arow++) {

      // std::cout << "processing row : " << row << std::endl;
      std::string arow_s = to_string(arow);
      matrix.MatrixDACConfig->SetParameter("writedac" + arow_s, 0);
      matrix.MatrixDACConfig->SetParameter("unused" + arow_s, 0);
      matrix.MatrixDACConfig->SetParameter("rowinjection" + arow_s, 0);
      matrix.MatrixDACConfig->SetParameter("analogbuffer" + arow_s, 0);
    };
    // this->ProgramSR(matrix);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 1);
    this->ProgramSR(matrix);
    matrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
    this->ProgramSR(matrix);
  };
}

// injections

void ATLASPixDevice::SetPixelInjection(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj_state) {
  std::string col_s;
  int double_col = 0;

  if((theMatrix.flavor == ATLASPix1Flavor::M1) || (theMatrix.flavor == ATLASPix1Flavor::M1Iso)) {
    std::string s = to_string(col);

    if(row < 200) {
      theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      // theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]);   // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);

    } else {
      // std::cout << "up pixels" << std::endl;
      theMatrix.MatrixDACConfig->SetParameter("RamUp" + s, theMatrix.TDAC[col][row]); // 0b1011
      // theMatrix.MatrixDACConfig->SetParameter("RamDown" + s, theMatrix.TDAC[col][row]); // 0b1011
      theMatrix.MatrixDACConfig->SetParameter("colinjDown" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("unusedDown" + s, 0);
      theMatrix.MatrixDACConfig->SetParameter("colinjUp" + s, inj_state);
      theMatrix.MatrixDACConfig->SetParameter("hitbusUp" + s, hb_state);
      theMatrix.MatrixDACConfig->SetParameter("unusedUp" + s, 0);
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
  theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  theMatrix.MatrixDACConfig->SetParameter("unused" + row_s, 0);
  theMatrix.MatrixDACConfig->SetParameter("rowinjection" + row_s, inj_state);
  theMatrix.MatrixDACConfig->SetParameter("analogbuffer" + row_s, ana_state);
  this->ProgramSR(theMatrix);
  theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  this->ProgramSR(theMatrix);
}

void ATLASPixDevice::SetInjectionOff() {

  for(uint32_t col = 0; col < theMatrix.ncol; col++) {
    // for(int row = 0; row < theMatrix.nrow; row++) {
    if(col % 5 == 0) {
      LOG(INFO) << "Setting col " << col << std::endl;
    }

    for(uint32_t row = 0; row < theMatrix.nrow; row++) {
      this->SetPixelInjectionState(col, row, 0, 0, 0);
    }
    this->ProgramSR(theMatrix);
    this->ResetWriteDAC();
    this->ProgramSR(theMatrix);
  }
  //}

  this->ReapplyMask();
}

void ATLASPixDevice::SetPixelInjectionState(uint32_t col, uint32_t row, bool ana_state, bool hb_state, bool inj) {
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

void ATLASPixDevice::ResetWriteDAC() {

  for(uint32_t row = 0; row < theMatrix.nrow; row++) {
    std::string row_s = to_string(row);
    theMatrix.MatrixDACConfig->SetParameter("writedac" + row_s, 0);
  }
}

void ATLASPixDevice::SetInjectionMask(uint32_t maskx, uint32_t masky, uint32_t state) {

  for(uint32_t col = 0; col < (theMatrix.ncol); col++) {
    if(((col + maskx) % theMatrix.maskx) == 0) {

      // LOG(INFO) << "injecting in col " << col << std::endl;

      this->SetPixelInjectionState(col, 0, 0, 0, state);
    }
  }

  for(uint32_t row = 0; row < theMatrix.nrow; row++) {
    if(((row + masky) % theMatrix.masky) == 0) {
      this->SetPixelInjectionState(0, row, 0, 0, state);
      // LOG(INFO) << "injecting in row " << row << std::endl;
    }
  };

  this->ProgramSR(theMatrix);
  this->ResetWriteDAC();
  this->ProgramSR(theMatrix);
}

std::vector<pixelhit>
ATLASPixDevice::CountHits(std::vector<pixelhit> data, uint32_t maskidx, uint32_t maskidy, CounterMap& counts) {

  for(auto& hit : data) {
    if((((hit.col + maskidx) % theMatrix.maskx) == 0) && (((hit.row + maskidy) % theMatrix.masky) == 0)) {
      counts[std::make_pair(hit.col, hit.row)]++;
    }
  }
  std::vector<pixelhit> hp;
  for(auto& cnt : counts) {
    if(cnt.second > 250) {
      // std::cout << "HOT PIXEL: " << cnt.first.first << " " << cnt.first.second << std::endl;
      // this->MaskPixel(cnt.first.first, cnt.first.second);
      pixelhit ahp;
      ahp.col = cnt.first.first;
      ahp.row = cnt.first.second;
      hp.push_back(ahp);
    }
  }

  return hp;
}

uint32_t ATLASPixDevice::CountHits(std::vector<pixelhit> data, uint32_t col, uint32_t row) {

  uint32_t count = 0;
  for(auto& hit : data) {
    // LOG(INFO) << hit.col << " " << hit.row << std::endl;
    if((hit.col == col) && (hit.row == row)) {
      count++;
    }
  }
  return count;
}

void ATLASPixDevice::AverageTOT(std::vector<pixelhit> data, uint32_t maskidx, uint32_t maskidy, TOTMap& tots) {

  CounterMap counts;

  for(auto& hit : data) {
    if((((hit.col + maskidx) % theMatrix.maskx) == 0) && (((hit.row + maskidy) % theMatrix.masky) == 0)) {
      counts[std::make_pair(hit.col, hit.row)]++;
      tots[std::make_pair(hit.col, hit.row)] += hit.tot;
    }
  }

  std::vector<pixelhit> hp;
  for(auto& cnt : counts) {
    tots[std::make_pair(cnt.first.first, cnt.first.second)] /= cnt.second;
  }
}

void ATLASPixDevice::doSCurvePixel(
  uint32_t col, uint32_t row, double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);

  std::vector<uint32_t> counts;
  std::vector<CounterMap> SCurveData(npoints);
  std::vector<pixelhit> hp;
  uint32_t count = 0;
  vinj = vmin;
  this->SetPixelInjection(col, row, 0, 0, 1);
  for(uint32_t i = 0; i < npoints; i++) {
    // LOG(INFO) << "pulse height : " << vinj << std::endl;
    this->pulse(npulses, 10000, 10000, vinj);
    usleep(1000);
    count = this->CountHits(this->getDataTimer(30), col, row);
    counts.push_back(count);
    LOG(INFO) << vinj << " " << count << std::endl;
    this->resetFIFO();
    vinj += dv;
  }
  this->SetPixelInjection(col, row, 0, 0, 0);
}

void ATLASPixDevice::doSCurves(double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);

  std::vector<CounterMap> SCurveData(npoints);
  std::vector<pixelhit> hp;
  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/SCURVE_VNDAC" + std::to_string(theMatrix.CurrentDACConfig->GetParameter("VNDACPix")) +
              "_TDAC" + std::to_string(theMatrix.TDAC[0][0] >> 1) + ".txt",
            std::ios::out);
  // disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  for(int mx = 0; mx < theMatrix.maskx; mx++) {
    for(int my = 0; my < theMatrix.masky; my++) {

      this->SetInjectionMask(mx, my, 1);
      this->reset();
      usleep(1000);
      //      for(auto& pix : hp) {
      //        this->MaskPixel(pix.col, pix.row);
      //      }

      vinj = vmin;
      for(uint32_t i = 0; i < npoints; i++) {
        LOG(INFO) << "pulse height : " << vinj << std::endl;
        this->pulse(npulses, 10000, 10000, vinj);
        usleep(10000);
        hp = this->CountHits(this->getDataTimer(200, true), mx, my, SCurveData[i]);
        this->resetFIFO();
        vinj += dv;
      }

      for(uint32_t col = 0; col < theMatrix.ncol; col++) {
        for(uint32_t row = 0; row < theMatrix.nrow; row++) {

          if((((col + mx) % theMatrix.maskx) == 0) && (((row + my) % theMatrix.masky) == 0)) {
            disk << col << " " << row << " ";
            for(uint32_t i = 0; i < npoints; i++) {
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

void ATLASPixDevice::PulseTune(double /*target*/) {

  // this->setPulse(theMatrix, 100, 10000, 10000, target);

  for(uint32_t col = 0; col < theMatrix.ncol; col++) {
    for(uint32_t row = 0; row < theMatrix.nrow; row++) {

      LOG(INFO) << "pixel " << col << " " << row << std::endl;
      bool ok = false;
      int curr_tdac = 4;
      int niter = 0;
      this->writeOneTDAC(theMatrix, col, row, curr_tdac);

      while(!ok && niter < 8) {

        uint32_t count = 0;
        this->SetPixelInjection(col, row, 0, 0, 1);
        this->pulse(100, 10000, 10000, 0.3);
        usleep(1000);
        count = this->CountHits(this->getDataTimer(300), col, row);
        this->resetFIFO();
        double ratio = double(count) / 100;
        LOG(INFO) << "TDAC " << curr_tdac << " " << ratio << std::endl;
        if(ratio < 0.45) {
          curr_tdac--;
        } else if(ratio > 0.55) {
          curr_tdac++;
        } else {
          ok = true;
        }

        if(curr_tdac < 0) {
          curr_tdac = 0;
          ok = true;
        }
        if(curr_tdac > 7) {
          curr_tdac = 7;
          ok = true;
        }

        this->writeOneTDAC(theMatrix, col, row, curr_tdac);
        this->SetPixelInjection(col, row, 0, 0, 0);

        niter++;
      }

      LOG(INFO) << "Best TDAC " << curr_tdac << std::endl;
    }
  }
}

void ATLASPixDevice::MeasureTOT(double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);

  std::vector<TOTMap> SCurveData(npoints);
  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/TOT_VNFBPix" + std::to_string(theMatrix.CurrentDACConfig->GetParameter("VNFBPix")) +
              "_VNPix" + std::to_string(theMatrix.CurrentDACConfig->GetParameter("VNPix")) + ".txt",
            std::ios::out);
  // disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  for(int mx = 0; mx < theMatrix.maskx; mx++) {
    for(int my = 0; my < theMatrix.masky; my++) {

      this->SetInjectionMask(mx, my, 1);

      vinj = vmin;
      for(uint32_t i = 0; i < npoints; i++) {
        LOG(INFO) << "pulse height : " << vinj << std::endl;
        this->pulse(npulses, 10000, 10000, vinj);
        this->AverageTOT(this->getDataTOvector(), mx, my, SCurveData[i]);
        // this->reset();
        vinj += dv;
      }

      for(uint32_t col = 0; col < theMatrix.ncol; col++) {
        for(uint32_t row = 0; row < theMatrix.nrow; row++) {

          if((((col + mx) % theMatrix.maskx) == 0) && (((row + my) % theMatrix.masky) == 0)) {
            disk << col << " " << row << " ";
            for(uint32_t i = 0; i < npoints; i++) {
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

void ATLASPixDevice::isLocked() {
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);

  if((*fifo_status >> 5) & 0b1) {
    std::cout << "yes" << std::endl;
  } else {
    std::cout << "no" << std::endl;
  }
}

uint32_t ATLASPixDevice::getTriggerCounter() {
  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* trg_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x18);
  return *trg_cnt;
}

void ATLASPixDevice::getTriggerCount() {
  LOG(INFO) << "Trigger accepted by FSM       " << this->getTriggerCounter() << std::endl;
  LOG(INFO) << "Trigger accepted by FSM (ext) " << this->readCounter(2) << std::endl;
  LOG(INFO) << "Trigger received              " << this->readCounter(3) << std::endl;
}

std::vector<uint32_t> ATLASPixDevice::getRawData() {
  if(data_type != "raw") {
    throw NoDataAvailable();
  }

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);

  uint32_t dataRead;
  std::vector<uint32_t> rawDataVec;

  dataRead = static_cast<uint32_t>(*data);
  // if there is data
  if(dataRead != 0) {
    // if filter for WEIRD_DATA is set, and the data has a WEIRD_DATA header, do not store.
    // Otherwise store data to return vector
    if(!(filter_weird_data && (dataRead >> 24 == 0b00000100))) {
      rawDataVec.push_back(dataRead);
    }
  }
  return rawDataVec;
}

pearydata ATLASPixDevice::getDataBin() {

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

  volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);

  make_directories(_output_directory);
  // write actual configuration
  theMatrix.writeGlobal(_output_directory + "/config.cfg");
  theMatrix.writeTDAC(_output_directory + "/config_TDAC.cfg");

  std::ofstream disk;
  disk.open(_output_directory + "/data.bin", std::ios::out | std::ios::binary);
  if(!disk.is_open()) {
    LOG(WARNING) << "Output data file NOT opened!";
  }

  uint32_t d1;

  while(true) {

    // check for stop request from another thread
    if(!this->_daqContinue.test_and_set()) {
      LOG(DEBUG) << "Exiting DAQ thread";
      break;
    }
    // check for new data in fifo
    d1 = static_cast<uint32_t>(*data);
    if(d1 == 0) {
      continue;
    } else {
      disk.write((char*)&d1, sizeof(uint32_t));
      disk.flush();
    }
  }

  disk.close();

  pearydata dummy;
  return dummy;
}

pearydata ATLASPixDevice::getData() {

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
  disk << "X:	Y:	   TS1:	   TS2: 	TOT:FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

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

    // HIT data of bit 31 is = 1
    if((d1 >> 31) == 1) {

      pixelhit hit = decodeHit(d1, theMatrix.CurrentDACConfig->GetParameter("ckdivend2"), gray_decoding_state);

      double timing = (((hit.ts1 * 2) - (fpga_ts_last)) & 0b11111111111);

      if(filter_hp) {
        if(std::find(hplist.begin(), hplist.end(), hit) == hplist.end()) {
          disk << "HIT " << hit.col << "	" << hit.row << "	" << hit.ts1 << "	" << hit.ts2 << "	" << hit.tot << "	"
               << fpga_ts_last << "	"
               << " " << TrCNT << " " << ((timestamp >> 8) & 0xFFFF) << " " << timing << std::endl;
        }

      } else {

        disk << "HIT " << hit.col << "	" << hit.row << "	" << hit.ts1 << "	" << hit.ts2 << "	" << hit.tot << "	"
             << fpga_ts_last << "	"
             << " " << TrCNT << " " << ((timestamp >> 8) & 0xFFFF) << " " << timing << std::endl;
      }
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
        disk << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        fpga_ts_last = fpga_ts;
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        fpga_ts_busy = d1 & 0xFFFFFF;
        disk << "BUSY_ASSERTED " << fpga_ts_busy << std::endl;
        break;
      case 0b00001100: // SERDES lock lost
        disk << "SERDES_LOCK_LOST" << std::endl;
        break;
      case 0b00001000: // SERDES lock established
        disk << "SERDES_LOCK_ESTABLISHED" << std::endl;
        break;
      case 0b00000100: // Unexpected/weird data came
        if(!filter_weird_data) {
          disk << "WEIRD_DATA " << std::hex << d1 << std::dec << std::endl;
        }
        break;
      default: // weird stuff, should not happend
        LOG(WARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
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

pearydata ATLASPixDevice::getDataTO(int /* maskx */, int /* masky */) {

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
      LOG(WARNING) << "stopping data taking because of noise (over 1M hits), re-applying mask" << std::endl;
      this->ReapplyMask();
      this->reset();
      sleep(1);
      break;
    }

    uint32_t d1 = static_cast<uint32_t>(*data);
    // std::cout << std::bitset<32>(d1) << std::endl;

    // HIT data of bit 31 is = 1
    if((d1 >> 31) == 1) {

      pixelhit hit = decodeHit(d1, theMatrix.CurrentDACConfig->GetParameter("ckdivend2"), gray_decoding_state);
      // LOG(INFO) << hit.col <<" " << hit.row << " " << hit.ts1 << ' ' << hit.ts2 << std::endl;
      disk << "HIT " << hit.col << "	" << hit.row << "	" << hit.ts1 << "	" << hit.ts2 << "   " << hit.tot << "	"
           << fpga_ts_last << "	"
           << " " << TrCNT << " " << ((timestamp >> 8) & 0xFFFF) << std::endl;
      // disk << std::bitset<32>(d1) << std::endl;
    }

    else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      // Parse the different data types (BUFFEROVERFLOW,TRIGGER,BUSY_ASSERTED)
      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        timestamp = d1 & 0xFFFFFF;
        // disk << "BINCOUNTER " << timestamp << std::endl;
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
        // LOG(INFO) << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        disk << "TRIGGER " << TrCNT << " " << fpga_ts << std::endl;
        fpga_ts_last = fpga_ts;
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        fpga_ts_busy = d1 & 0xFFFFFF;
        disk << "BUSY_ASSERTED " << fpga_ts_busy << std::endl;
        break;
      case 0b00001100: // SERDES lock lost
        disk << "SERDES_LOCK_LOST" << std::endl;
        break;
      case 0b00001000: // SERDES lock established
        disk << "SERDES_LOCK_ESTABLISHED" << std::endl;
        break;
      case 0b00000100: // Unexpected/weird data came
        // sdisk << "WEIRD_DATA" << std::endl;
        break;
      default: // weird stuff, should not happend
        LOG(WARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }

  disk.close();

  LOG(INFO) << "data count : " << datatocnt << std::endl;

  pearydata dummy;
  return dummy;
}

std::vector<pixelhit> ATLASPixDevice::getDataTOvector(uint32_t /* timeout */, bool /* noisescan */) {

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
  uint32_t TrCNT = 0;

  bool to = false;
  uint32_t tocnt = 0;
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

    if(datacnt > 1e5) {
      break;
    }

    uint32_t d1 = static_cast<uint32_t>(*data);

    // HIT data of bit 31 is = 1
    pixelhit hit = decodeHit(d1, theMatrix.CurrentDACConfig->GetParameter("ckdivend2"), gray_decoding_state);

    if((d1 >> 31) == 1) {
      if(filter_hp) {
        if(std::find(hplist.begin(), hplist.end(), hit) == hplist.end()) {
          datavec.push_back(hit);
          datacnt++;
        }
      }
    } else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      // Parse the different data types (BUFFEROVERFLOW,TRIGGER,BUSY_ASSERTED)
      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        break;
      case 0b00000001: // Buffer overflow, data after this are lost
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
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        break;
      case 0b00001100: // SERDES lock lost
        break;
      case 0b00001000: // SERDES lock established
        break;
      case 0b00000100: // Unexpected/weird data came
        break;
      default: // weird stuff, should not happend
        LOG(WARNING) << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }

  LOG(INFO) << "data count : " << datacnt << std::endl;

  return datavec;
}

std::vector<pixelhit> ATLASPixDevice::getDataTimer(uint32_t timeout, bool to_nodata) {

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
  uint32_t TrCNT = 0;

  bool to = false;
  uint32_t tocnt = 0;
  std::vector<pixelhit> datavec;
  uint32_t datacnt = 0;

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  while(to == false) {

    // check for stop request from another thread
    if(!this->_daqContinue.test_and_set())
      break;

    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    if(dur.count() > timeout) {
      break;
    }
    // check for new first half-word or restart loop

    if((*fifo_status & 0x1) == 0) {
      usleep(1);
      tocnt += 1;
      if(tocnt == Tuning_timeout && to_nodata) {
        to = true;
        break;
      } else {
        continue;
      }
    }

    uint32_t d1 = static_cast<uint32_t>(*data);
    // std::cout << std::bitset<32>(d1) << std::endl;

    // HIT data of bit 31 is = 1
    pixelhit hit = decodeHit(d1, theMatrix.CurrentDACConfig->GetParameter("ckdivend2"), gray_decoding_state);

    if((d1 >> 31) == 1) {
      if(filter_hp) {
        if(std::find(hplist.begin(), hplist.end(), hit) == hplist.end()) {
          datavec.push_back(hit);
          datacnt++;
        }
      }
    } else {

      uint32_t data_type = (d1 >> 24) & 0xFF;

      // Parse the different data types (BUFFEROVERFLOW,TRIGGER,BUSY_ASSERTED)
      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        break;
      case 0b00000001: // Buffer overflow, data after this are lost
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
        fpga_ts = 0;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        break;
      case 0b00001100: // SERDES lock lost
        break;
      case 0b00001000: // SERDES lock established
        break;
      case 0b00000100: // Unexpected/weird data came
        break;
      default: // weird stuff, should not happend
        break;
      }
    }
  }

  // LOG(INFO) << "data count : " << datacnt << std::endl;

  return datavec;
}

void ATLASPixDevice::ReapplyMask() {

  LOG(INFO) << "re-applying mask " << std::endl;
  for(uint32_t col = 0; col < theMatrix.ncol; col++) {
    for(uint32_t row = 0; row < theMatrix.nrow; row++) {
      if(theMatrix.MASK[col][row] == 1) {
        // LOG(INFO) << "masking " << col << " " << row << std::endl;
        this->MaskPixel(col, row);
      }
    };
  };
}

void ATLASPixDevice::dataTuning(double vmax, int nstep, uint32_t npulses) {

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

  LOG(INFO) << "Tuning using data for target " << vmax;
  for(uint32_t col = 0; col < theMatrix.ncol; col++) {
    for(uint32_t row = 0; row < theMatrix.nrow; row++) {

      int cur_tdac = 4;
      bool done = false;
      if(nstep == 0)
        done = true;
      uint32_t cnt = 0;
      uint32_t loop = 0;
      bool masked = false;

      this->setPulse(theMatrix, npulses, 10000, 10000, vmax);
      // this->SetPixelInjection(col,row,1,1);
      LOG(INFO) << rev << red << "Pixel  " << col << " " << row << reset << std::endl;

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
            // LOG(INFO) << "Pixel  " << hit.col<< " " << hit.row << std::endl;
          }
        }

        LOG(INFO) << "tdac: " << cur_tdac << " "
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
      // LOG(INFO) << "Pixel  " << col << " " << row << " tdac: " << cur_tdac << " " << "cnt: " << cnt << std::endl;
      for(auto hit : data) {
        if(hit.col == col && hit.row == row) {
          cnt++;
          // LOG(INFO) << "Pixel  " << hit.col<< " " << hit.row << std::endl;
        }
      }
      disk << col << " " << row << " " << cur_tdac << " " << cnt << std::endl;
      this->SetPixelInjection(col, row, 0, 0, 0);

      LOG(INFO) << rev << green << " tdac: " << cur_tdac << " "
                << "mask: " << masked << " cnt: " << cnt << reset << std::endl;
    }
  }

  disk.close();
}

void ATLASPixDevice::VerifyTuning(double vmin, double vmax, int npulses, int npoints) {
  double vinj = vmin;
  double dv = (vmax - vmin) / (npoints - 1);

  std::vector<CounterMap> SCurveData(npoints);
  std::vector<pixelhit> hp;
  make_directories(_output_directory);
  std::ofstream disk;
  disk.open(_output_directory + "/SCURVE_TDAC_" + "verification" + ".txt", std::ios::out);
  // disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:  TR_CNT:  BinCounter :  " << std::endl;

  for(int mx = 0; mx < theMatrix.maskx; mx++) {
    for(int my = 0; my < theMatrix.masky; my++) {

      this->SetInjectionMask(mx, my, 1);
      this->reset();
      usleep(1000);
      //      for(auto& pix : hp) {
      //        this->MaskPixel(pix.col, pix.row);
      //      }

      vinj = vmin;
      for(int i = 0; i < npoints; i++) {
        LOG(INFO) << "pulse height : " << vinj << std::endl;
        this->pulse(npulses, 10000, 10000, vinj);
        usleep(10000);
        hp = this->CountHits(this->getDataTimer(50), mx, my, SCurveData[i]);
        this->resetFIFO();
        vinj += dv;
      }

      for(uint32_t col = 0; col < theMatrix.ncol; col++) {
        for(uint32_t row = 0; row < theMatrix.nrow; row++) {

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

void ATLASPixDevice::doSCurvesAndWrite(
  std::string basefolder, double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

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

  for(uint32_t col = 0; col < theMatrix.ncol; col++) {
    for(uint32_t row = 0; row < theMatrix.nrow; row++) {

      // start = std::clock();

      if(row % 5 == 0) {
        std::cout << "X: " << col << " Y: " << row << "\n";
      }

      vinj = vmin;
      // this->SetPixelInjection(theMatrix,0,0,1,1);
      // this->SetPixelInjection(theMatrix,0,0,0,0);

      this->SetPixelInjection(col, row, 1, 1, 1);
      this->resetCounters();

      for(uint32_t i = 0; i < npoints; i++) {
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

void ATLASPixDevice::TDACScan(int VNDAC, double vmin, double vmax, uint32_t npulses, uint32_t npoints) {

  theMatrix.CurrentDACConfig->SetParameter("VNDACPix", VNDAC);
  this->ProgramSR(theMatrix);

  for(int tdac = 0; tdac <= 7; tdac += 1) {

    this->setAllTDAC(tdac);
    this->doSCurves(vmin, vmax, npulses, npoints);
  }
}

// CaR Board related

void ATLASPixDevice::reset() {
  // LOG(INFO) << "Resetting " << DEVICE_NAME;

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

  // Locking on comma word from ATLASPix
  theMatrix.CurrentDACConfig->SetParameter("RO_res_n", 0);
  this->ProgramSR(theMatrix);

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b10000;
  usleep(50);
  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b00000;

  usleep(100);

  theMatrix.CurrentDACConfig->SetParameter("RO_res_n", 1);
  this->ProgramSR(theMatrix);
}

void ATLASPixDevice::resetFIFO() {
  // LOG(INFO) << "Resetting " << DEVICE_NAME;

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b10000;
  usleep(50);
  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b00000;
}

std::string ATLASPixDevice::getName() {
  return DEVICE_NAME;
}

void ATLASPixDevice::powerUp() {
  LOG(INFO) << DEVICE_NAME << ": Powering up ATLASPix";
  std::cout << '\n';

  this->setVoltage("VCC25", _config.Get("vcc25", ATLASPix_VCC25), _config.Get("vcc25_current", ATLASPix_VCC25_CURRENT));
  this->switchOn("VCC25");

  usleep(200000);

  this->setVoltage("VDDD", _config.Get("vddd", ATLASPix_VDDD), _config.Get("vddd_current", ATLASPix_VDDD_CURRENT));
  this->switchOn("VDDD");

  usleep(200000);

  this->setVoltage("VDDRam", _config.Get("vddram", ATLASPix_VDDRam), _config.Get("vddram_current", ATLASPix_VDDRam_CURRENT));
  this->switchOn("VDDRam");

  usleep(200000);

  this->setVoltage(
    "VDDHigh", _config.Get("vddhigh", ATLASPix_VDDHigh), _config.Get("vddhigh_current", ATLASPix_VDDHigh_CURRENT));
  this->switchOn("VDDHigh");

  usleep(200000);

  this->setVoltage("VDDA", _config.Get("vdda", ATLASPix_VDDA), _config.Get("vdda_current", ATLASPix_VDDA_CURRENT));
  this->switchOn("VDDA");

  usleep(200000);

  this->setVoltage("VSSA", _config.Get("vssa", ATLASPix_VSSA), _config.Get("vssa_current", ATLASPix_VSSA_CURRENT));
  this->switchOn("VSSA");

  // Analog biases

  this->setVoltage("GNDDACPix", theMatrix.GNDDACPix);
  this->switchOn("GNDDACPix");

  this->setVoltage("VMinusPix", theMatrix.VMINUSPix);
  this->switchOn("VMinusPix");

  this->setVoltage("GatePix", theMatrix.GatePix);
  this->switchOn("GatePix");

  this->setVoltage("VNFBPix", theMatrix.VNFBPix);
  this->switchOn("VNFBPix");

  this->setVoltage("BLResPix", theMatrix.BLResPix);
  this->switchOn("BLResPix");

  this->setVoltage("VMain2", theMatrix.VMain2);
  this->switchOn("VMain2");

  // Threshold and Baseline

  this->setVoltage("VThPix", theMatrix.ThPix);
  this->switchOn("VThPix");

  this->setVoltage("VBLPix", theMatrix.BLPix);
  this->switchOn("VBLPix");
}

void ATLASPixDevice::powerDown() {
  LOG(INFO) << DEVICE_NAME << ": Power off ATLASPix";

  LOG(DEBUG) << "Powering off VDDA";
  this->switchOff("VDDA");

  LOG(DEBUG) << "Powering off VDDD";
  this->switchOff("VDDD");

  LOG(DEBUG) << "Powering off VSSA";
  this->switchOff("VSSA");

  LOG(DEBUG) << "Powering off VDDRam";
  this->switchOff("VDDRam");

  LOG(DEBUG) << "Powering off VDDHigh";
  this->switchOff("VDDHigh");

  LOG(DEBUG) << "Powering off VCC25";
  this->switchOff("VCC25");

  LOG(DEBUG) << "Turning off GNDDacPix";
  this->switchOff("GNDDACPix");

  LOG(DEBUG) << "Turning off VMinusPix_M1";
  this->switchOff("VMinusPix");

  LOG(DEBUG) << "Turning off GatePix_M1";
  this->switchOff("GatePix");
}

// daq thread implementation
namespace {
  using TimeoutClock = std::chrono::steady_clock;
  using TimeoutDuration = std::chrono::steady_clock::duration;
  using TimeoutTimepoint = std::chrono::steady_clock::time_point;
} // unnamed namespace

void ATLASPixDevice::NoiseRun(double duration) {

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  this->daqStart();

  while(1) {
    auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
    if(dur.count() > duration)
      // throw DeviceException("Cannot lock to external clock.");
      break;
  }

  this->daqStop();
}

void ATLASPixDevice::daqStart() {
  // ensure only one daq thread is running
  if(daqRunning) {
    LOG(WARNING) << "Data aquisition is already running";
    return;
  }

  void* readout_base =
    _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
  volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

  if(_daqThread.joinable()) {
    LOG(WARNING) << "DAQ thread is already running";
    return;
  }

  daqRunning = true;

  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b10000;
  usleep(50);
  *fifo_config = (*fifo_config & 0xFFFFFFEF) + 0b00000;

  // arm the stop flag and start running
  this->resetCounters();

  if(data_type != "raw") {
    _daqContinue.test_and_set();
    _daqThread = std::thread(&ATLASPixDevice::runDaq, this);
  }
  // LOG(INFO) << "acquisition started" << std::endl;
}

void ATLASPixDevice::daqStop() {
  if(_daqThread.joinable()) {
    // signal to daq thread that we want to stop and wait until it does
    _daqContinue.clear();
    _daqThread.join();
    // LOG(INFO) << "Trigger count at end of run : " << this->getTriggerCounter() << std::endl;
  }
  _daqContinue.clear();
  daqRunning = false;
}

void ATLASPixDevice::runDaq() {

  if(data_type == "binary") {
    getDataBin();
  } else if(data_type == "text") {
    getData();
  } else {
    LOG(WARNING) << "Unknown output format \"" << data_type << "\"";
    return;
  }
}

void ATLASPixDevice::powerStatusLog() {
  LOG(INFO) << DEVICE_NAME << " power status:";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_4) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_4) << "A";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";

  LOG(INFO) << "VSSA:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_2) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_2) << "A";

  LOG(INFO) << "VDDRam:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_1) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_1) << "A";

  LOG(INFO) << "VCC25:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_5) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_5) << "A";

  LOG(INFO) << "VDDHigh:";
  LOG(INFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_6) << "V";
  LOG(INFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_6) << "A";
}

void ATLASPixDevice::MonitorPower() {

  if(_monitorPowerThread.joinable()) {
    LOG(WARNING) << "Power monitoring is already running";
    return;
  }

  _monitorPowerContinue.test_and_set();
  _monitorPowerThread = std::thread(&ATLASPixDevice::runMonitorPower, this);
}

void ATLASPixDevice::StopMonitorPower() {

  _monitorPowerContinue.clear();
  _monitorPowerThread.join();
}

void ATLASPixDevice::runMonitorPower() {

  make_directories(_output_directory);
  std::ofstream outFile(_output_directory + "/power_log.txt");
  if(!outFile.is_open()) {
    _monitorPowerContinue.clear();
    // TODO log with error
    return;
  }

  outFile << "# VDDD voltage [V], VDDD current [A], VDDA voltage [V], VDDA current [A], VSSA voltage [V], VSSA current [A]"
          << std::endl;
  while(true) {
    // check for stop request from another thread
    if(!this->_monitorPowerContinue.test_and_set())
      break;

    outFile << _hal->measureVoltage(PWR_OUT_4) << " " << _hal->measureCurrent(PWR_OUT_4) << " "
            << _hal->measureVoltage(PWR_OUT_3) << " " << _hal->measureCurrent(PWR_OUT_3) << " "
            << _hal->measureVoltage(PWR_OUT_2) << " " << _hal->measureCurrent(PWR_OUT_2) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  outFile.close();
}

void ATLASPixDevice::WriteFWRegistersAndBias(std::string name) {
  std::ofstream cfg(name, std::ofstream::out | std::ofstream::app);

  cfg << "ro_enable : " << std::left << std::setw(20) << ro_enable << std::endl;
  cfg << "edge_sel : " << std::left << std::setw(20) << edge_sel << std::endl;
  cfg << "trigger_enable : " << std::left << std::setw(20) << trigger_enable << std::endl;
  cfg << "trigger_always_armed : " << std::left << std::setw(20) << trigger_always_armed << std::endl;
  cfg << "busy_when_armed : " << std::left << std::setw(20) << busy_when_armed << std::endl;
  cfg << "t0_enable : " << std::left << std::setw(20) << t0_enable << std::endl;
  cfg << "send_fpga_ts : " << std::left << std::setw(20) << send_fpga_ts << std::endl;
  cfg << "armduration : " << std::left << std::setw(20) << armduration << std::endl;
  cfg << "filter_hp : " << std::left << std::setw(20) << filter_hp << std::endl;
  cfg << "HW_masking : " << std::left << std::setw(20) << HW_masking << std::endl;
  cfg << "filter_weird_data : " << std::left << std::setw(20) << filter_weird_data << std::endl;
  cfg << "trigger_injection : " << std::left << std::setw(20) << trigger_injection << std::endl;
  cfg << "gray_decode : " << std::left << std::setw(20) << gray_decode << std::endl;
  cfg << "VSSA : " << std::left << std::setw(20) << _hal->measureVoltage(PWR_OUT_2) << std::endl;
  cfg << "VDDD : " << std::left << std::setw(20) << _hal->measureVoltage(PWR_OUT_4) << std::endl;
  cfg << "VDDA : " << std::left << std::setw(20) << _hal->measureVoltage(PWR_OUT_3) << std::endl;
  cfg << "VDDRam : " << std::left << std::setw(20) << _hal->measureVoltage(PWR_OUT_1) << std::endl;
}

void ATLASPixDevice::WriteConfig(std::string name) {
  make_directories(_output_directory);
  theMatrix.writeGlobal(_output_directory + "/" + name + ".cfg");
  theMatrix.writeTDAC(_output_directory + "/" + name + "_TDAC.cfg");
  this->WriteFWRegistersAndBias(_output_directory + "/" + name + ".cfg");
}

void ATLASPixDevice::LoadConfig(std::string basename) {
  theMatrix.loadGlobal(basename + ".cfg");
  this->ProgramSR(theMatrix);
  // 2018-02-14 msmk:
  // not sure if this is correct, but the previous version did a manual
  // power up here as well. Could this be replaced by a call
  // to powerUp directly? Is this the intended functionality, i.e.
  // this loads configuration data from file and powers everything up or
  // should this be just the loading which must be followed up by the actual
  // powerUp command?
  //  this->setVoltage("GNDDACPix", theMatrix.GNDDACPix);
  //  this->switchOn("GNDDACPix");
  //  this->setVoltage("VMinusPix", theMatrix.VMINUSPix);
  //  this->switchOn("VMinusPix");
  //  this->setVoltage("GatePix", theMatrix.GatePix);
  //  this->switchOn("GatePix");
  //  this->setVoltage("VBLPix", theMatrix.BLPix);
  //  this->switchOn("VBLPix");
  //  this->setVoltage("VThPix", theMatrix.ThPix);
  //  this->switchOn("VThPix");
  // this->LoadTDAC(basename + "_TDAC.cfg");
}
