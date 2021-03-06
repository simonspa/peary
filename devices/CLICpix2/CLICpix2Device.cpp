/**
 * Caribou Device implementation for CLICpix2
 */

#include "CLICpix2Device.hpp"
#include "clicpix2_utilities.hpp"
#include "utils/log.hpp"

#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <math.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace caribou;
using namespace clicpix2_utils;

CLICpix2Device::CLICpix2Device(const caribou::Configuration config)
    : CaribouDevice(config, std::string(DEFAULT_DEVICEPATH)), pg_total_length(0) {

  // Regsiter device-specific commands:
  _dispatcher.add("configureMatrix", &CLICpix2Device::configureMatrix, this);
  _dispatcher.add("configurePatternGenerator", &CLICpix2Device::configurePatternGenerator, this);
  _dispatcher.add("exploreInterface", &CLICpix2Device::exploreInterface, this);
  _dispatcher.add("powerStatusLog", &CLICpix2Device::powerStatusLog, this);
  _dispatcher.add("triggerPatternGenerator", &CLICpix2Device::triggerPatternGenerator, this);
  _dispatcher.add("configureClock", &CLICpix2Device::configureClock, this);
  _dispatcher.add("clearTimestamps", &CLICpix2Device::clearTimestamps, this);
  _dispatcher.add("setOutputMultiplexer", &CLICpix2Device::setOutputMultiplexer, this);
  _dispatcher.add("getMemory", &CLICpix2Device::getMem, this);
  _dispatcher.add("setMemory", &CLICpix2Device::setMem, this);

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_1);
  _periphery.add("vdda", PWR_OUT_3);
  _periphery.add("cmlbuffers_vdd", PWR_OUT_4);
  _periphery.add("vddcml", PWR_OUT_5);
  _periphery.add("cmlbuffers_vcco", PWR_OUT_7);

  _periphery.add("cml_iref", CUR_1);
  _periphery.add("dac_iref", CUR_2);

  _periphery.add("dac_out", VOL_IN_1);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(CLICPIX2_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(CLICPIX2_MEMORY);

  // set default CLICpix2 control
  setMemory("reset", 0);
}

void CLICpix2Device::getMem(std::string name) {
  auto value = getMemory(name);
  LOG(INFO) << name << " = 0x" << to_hex_string(value);
}

void CLICpix2Device::setMem(std::string name, uint32_t value) {
  setMemory(name, value);
}

void CLICpix2Device::configure() {
  LOG(INFO) << "Configuring";
  configureClock(_config.Get<bool>("clock_internal", false));
  reset();
  mDelay(10);

  // Read pattern generator from the configuration and program it:
  std::string pg = _config.Get("patterngenerator", "");
  if(!pg.empty()) {
    LOG(INFO) << "Found pattern generator in configuration, programming...";
    configurePatternGenerator(pg);
  } else {
    LOG(INFO) << "No pattern generator found in configuration.";
  }

  // Read matrix file from the configuration and program it:
  std::string matrix = _config.Get("matrix", "");
  if(!matrix.empty()) {
    LOG(INFO) << "Found pixel matrix setup in configuration, programming...";
    configureMatrix(matrix);
  } else {
    LOG(INFO) << "No pixel matrix configuration setting found.";
  }

  // FIXME set all DACs provided with config
  // Call the base class configuration function:
  CaribouDevice<iface_spi_CLICpix2>::configure();

  // If no matrix was given via the config, set a fully masked matrix:
  if(_config.Get("matrix", "").empty()) {
    configureMatrix();
  }
}

void CLICpix2Device::reset() {
  LOG(DEBUG) << "Resetting";

  // assert reset:
  setMemory("reset", getMemory("reset") & ~(CLICPIX2_CONTROL_RESET_MASK));
  usleep(1);
  // deny reset:
  setMemory("reset", getMemory("reset") | CLICPIX2_CONTROL_RESET_MASK);
}

CLICpix2Device::~CLICpix2Device() {

  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void CLICpix2Device::setSpecialRegister(std::string name, uint32_t value) {

  if(name == "threshold") {
    // Get threshold LSB and MSB
    uint8_t msb = floor(value / 121.) * 14;
    uint8_t lsb = (value % 121) + 64;
    uint32_t maxThl = ceil(255 / 14.) * 121;
    if(value >= maxThl) {
      msb = 255;
      lsb = 255;
      LOG(WARNING) << "Threshold range is limited to " << maxThl << ", setting 255-255";
    }
    LOG(DEBUG) << "Threshold lookup: " << value << " = " << static_cast<int>(msb) << "-" << static_cast<int>(lsb);
    // Set the two values:
    this->setRegister("threshold_msb", msb);
    this->setRegister("threshold_lsb", lsb);
  } else if(name == "test_cap_1") {
    uint8_t msb = floor(value / 121.) * 14;
    uint8_t lsb = (value % 121) + 84;
    uint32_t maxThl = ceil(255 / 14.) * 121;
    if(value >= maxThl) {
      msb = 255;
      lsb = 255;
      LOG(WARNING) << "Test_Cap_1 range is limited to " << maxThl << ", setting 255-255";
    }
    LOG(DEBUG) << "Test_cap_1 lookup: " << value << " = " << static_cast<int>(msb) << "-" << static_cast<int>(lsb);
    // Set the two values:
    this->setRegister("test_cap_1_msb", msb);
    this->setRegister("test_cap_1_lsb", lsb);
  } else if(name == "pulsegen_counts") {
    // Get pulsegen_counts LSB and MSB
    LOG(DEBUG) << "Pulsegen_counts lookup: " << value << " = " << static_cast<int>((value >> 8) & 0x00FF) << "-"
               << static_cast<int>(value & 0x00FF);
    // Set the two values:
    this->setRegister("pulsegen_counts_msb", (value >> 8) & 0x00FF);
    this->setRegister("pulsegen_counts_lsb", value & 0x00FF);
  } else if(name == "pulsegen_delay") {
    // Get pulsegen_counts LSB and MSB
    LOG(DEBUG) << "Pulsegen_delay lookup: " << value << " = " << static_cast<int>((value >> 8) & 0x00FF) << "-"
               << static_cast<int>(value & 0x00FF);
    // Set the two values:
    this->setRegister("pulsegen_delay_msb", (value >> 8) & 0x00FF);
    this->setRegister("pulsegen_delay_lsb", value & 0x00FF);
  } else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

void CLICpix2Device::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << " CMLBUFFERS_VDD";
  this->setVoltage("cmlbuffers_vdd",
                   _config.Get("cmlbuffers_vdd", CLICpix2_CMLBUFFERS_VDD),
                   _config.Get("cmlbuffers_vdd_current", CLICpix2_CMLBUFFERS_VDD_CURRENT));
  this->switchOn("cmlbuffers_vdd");

  LOG(DEBUG) << " CMLBUFFERS_VCCO";
  this->setVoltage("cmlbuffers_vcco",
                   _config.Get("cmlbuffers_vcco", CLICpix2_CMLBUFFERS_VCCO),
                   _config.Get("cmlbuffers_vcco_current", CLICpix2_CMLBUFFERS_VCCO_CURRENT));
  this->switchOn("cmlbuffers_vcco");

  LOG(DEBUG) << " VDDCML";
  this->setVoltage("vddcml", _config.Get("vddcml", CLICpix2_VDDCML), _config.Get("vddcml_current", CLICpix2_VDDCML_CURRENT));
  this->switchOn("vddcml");

  LOG(DEBUG) << " VDDD";
  this->setVoltage("vddd", _config.Get("vddd", CLICpix2_VDDD), _config.Get("vddd_current", CLICpix2_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(DEBUG) << " VDDA";
  this->setVoltage("vdda", _config.Get("vdda", CLICpix2_VDDA), _config.Get("vdda_current", CLICpix2_VDDA_CURRENT));
  this->switchOn("vdda");

  LOG(DEBUG) << " CML_IREF";
  this->setCurrent("cml_iref", _config.Get("cml_iref", CLICpix2_CML_IREF), CLICpix2_CML_IREF_POL);
  this->switchOn("cml_iref");

  // Only enable if present in the configuration:
  if(_config.Has("dac_iref")) {
    LOG(DEBUG) << " DAC_IREF";
    this->setCurrent("dac_iref", _config.Get("dac_iref", CLICpix2_DAC_IREF), CLICpix2_DAC_IREF_POL);
    this->switchOn("dac_iref");
  }
}

void CLICpix2Device::powerDown() {
  LOG(INFO) << "Power off";

  LOG(DEBUG) << "Power off CML_IREF";
  this->switchOff("cml_iref");

  LOG(DEBUG) << "Power off DAC_IREF";
  this->switchOff("dac_iref");

  LOG(DEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(DEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(DEBUG) << "Power off VDDCML";
  this->switchOff("vddcml");

  LOG(DEBUG) << "Power off CMLBUFFERS_VCCO";
  this->switchOff("cmlbuffers_vcco");

  LOG(DEBUG) << "Power off CMLBUFFERS_VDD";
  this->switchOff("cmlbuffers_vdd");
}

void CLICpix2Device::configureMatrix(std::string filename) {

  // Temporarily switch off any compression algorithm:
  uint32_t comp = this->getRegister("comp");
  uint32_t sp_comp = this->getRegister("sp_comp");
  this->setRegister("comp", 0);
  this->setRegister("sp_comp", 0);

  if(!filename.empty()) {
    LOG(DEBUG) << "Configuring the pixel matrix from file \"" << filename << "\"";
    pixelsConfig = readMatrix(filename);
  }

  // Prepare decoder for configuration:
  clicpix2_frameDecoder decoder(false, false, pixelsConfig);

  // Retry programming matrix:
  int retry = 0;
  int retry_max = _config.Get<int>("retry_matrix_config", 3);
  while(true) {
    try {
      programMatrix();

      // Read back the matrix configuration and thus clear it:
      LOG(DEBUG) << "Flushing matrix...";
      std::vector<uint32_t> frame = getFrame();
      decoder.decode(frame, false);

      LOG(INFO) << "Verifing matrix configuration...";
      bool configurationError = false;
      for(const auto& px : pixelsConfig) {

        // Fetch readback value for this pixel:
        pixelReadout pxv = decoder.get(px.first.first, px.first.second);

        // The flag bit if the readout is returned as (mask | (threshold & 0x1)), thus resetting to mask state only:
        if(pxv.GetBit(8)) {
          pxv.SetFlag(px.second.GetMask());
        }

        // Compare with value read from the matrix:
        if(px.second != pxv) {
          LOG(ERROR) << "Matrix configuration of pixel " << static_cast<int>(px.first.second) << ","
                     << static_cast<int>(px.first.first) << " does not match:";
          LOG(ERROR) << to_bit_string(px.second.GetLatches()) << " != " << to_bit_string(pxv.GetLatches());
          configurationError = true;
        }
      }

      if(configurationError) {
        throw DataException("Matrix configuration mismatch");
      }

      LOG(INFO) << "Verified matrix configuration.";
      break;
    } catch(caribou::DataException& e) {
      LOG(ERROR) << e.what();
      if(++retry == retry_max) {
        throw CommunicationError("Matrix configuration failed");
      }
      LOG(INFO) << "Repeating configuration attempt";
    }
  }

  // Reset compression state to previous values:
  this->setRegister("comp", comp);
  this->setRegister("sp_comp", sp_comp);
}

void CLICpix2Device::triggerPatternGenerator(bool sleep) {

  LOG(DEBUG) << "Triggering pattern generator once.";

  // Write into enable register of pattern generator:
  setMemory("wave_control", getMemory("wave_control") & ~(CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK));
  setMemory("wave_control", getMemory("wave_control") | CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK);

  // Wait for its length before returning:
  if(sleep)
    usleep(pg_total_length / 100);
}

void CLICpix2Device::configurePatternGenerator(std::string filename) {

  LOG(DEBUG) << "Programming pattern generator.";
  std::vector<uint32_t> patterns;

  std::ifstream pgfile(filename);
  if(!pgfile.is_open()) {
    LOG(ERROR) << "Could not open pattern generator configuration file \"" << filename << "\"";
    throw ConfigInvalid("Could not open pattern generator configuration file \"" + filename + "\"");
  }

  std::string line = "";
  pg_total_length = 0;
  while(std::getline(pgfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pgline(line);
    std::string signals;
    uint32_t duration;
    if(pgline >> signals >> duration) {
      uint32_t pattern = 0;

      std::stringstream ss(signals);
      while(ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        if(substr == "PWR") {
          pattern |= CLICPIX2_CONTROL_WAVE_GENERATOR_EVENTS_PWR_PULSE_MASK;
        } else if(substr == "TP") {
          pattern |= CLICPIX2_CONTROL_WAVE_GENERATOR_EVENTS_TP_MASK;
        } else if(substr == "SH") {
          pattern |= CLICPIX2_CONTROL_WAVE_GENERATOR_EVENTS_SHUTTER_MASK;
        } else if(substr == "NONE") {
          // Add nothing
        } else {
          LOG(ERROR) << "Unrecognized pattern for pattern generator: " << substr << " - ignoring.";
        }
      }
      pattern |= (duration & CLICPIX2_CONTROL_WAVE_GENERATOR_EVENTS_DURATION_MASK);
      LOG(DEBUG) << "PG signals: " << signals << " duration: " << duration << " pattern: " << to_bit_string(pattern);
      patterns.push_back(pattern);
      pg_total_length += duration;
    }
  }
  LOG(DEBUG) << "Now " << patterns.size() << " patterns cached.";
  LOG(DEBUG) << "Total length " << pg_total_length << " clk cycles.";
  if(patterns.size() > 32) {
    LOG(FATAL) << "Pattern generator contains too many statements, maximum allowed: 32.";
    throw ConfigInvalid("Pattern generator too long");
  }

  // Switch loop mode off:
  setMemory("wave_control", getMemory("wave_control") & ~(CLICPIX2_CONTROL_WAVE_GENERATOR_LOOP_MODE_MASK));
  for(size_t reg = 0; reg < patterns.size(); reg++) {
    setMemory("wave_events", (reg << 2), patterns.at(reg));
  }
  LOG(DEBUG) << "Done configuring pattern generator.";
}

void CLICpix2Device::programMatrix() {

  // Use a boolean vector to construct full matrix data array:
  std::vector<bool> matrix;

  // At the end of the column, add one flip-flop per double column:
  LOG(DEBUG) << "Add EOC bits";
  matrix.insert(matrix.end(), 64, 0);

  // Loop over all rows, start with lowest:
  for(size_t row = 0; row < 128; row++) {

    // After every superpixel (16 pixels), add one flip-flop per double column:
    if(row % 8 == 0) {
      matrix.insert(matrix.end(), 64, 0);
      LOG(DEBUG) << "Add superpixel flipflop for all double columns. (matrix: " << matrix.size() << "b)";
    }

    // Perform snake pattern within double column:
    for(size_t col = 0; col < 2; col++) {
      // Store 14 bit per pixel:
      for(int bit = 13; bit >= 0; --bit) {
        // Loop over all double columns
        for(size_t dcolumn = 0; dcolumn < 64; dcolumn++) {
          // Send one bit per double column to form one 64bit word
          pixelConfig px = pixelsConfig[std::make_pair(row, 2 * dcolumn + ((row + col) % 2))];
          matrix.push_back(px.GetBit(bit));
        }
      }
      LOG(DEBUG) << "Pixel row " << row << ((row + col) % 2 ? " (odd)" : " (even)")
                 << " done, matrix size: " << matrix.size() << "bit";
    }
  }
  LOG(DEBUG) << "Full matrix size: " << matrix.size() << "b";

  // At the very end, write one 64bit word with zeros to blank matrix after readout:
  matrix.insert(matrix.end(), 64, 0);
  LOG(DEBUG) << "Full matrix size incl. clear: " << matrix.size() << "b";

  std::vector<std::pair<typename iface_spi_CLICpix2::reg_type, typename iface_spi_CLICpix2::data_type>> spi_data;
  register_t<> reg = _registers.get("matrix_programming");

  // Read matrix in 8b chunks to send over SPI interface:
  uint8_t word = 0;
  for(size_t bit = 0; bit < matrix.size(); bit++) {
    word |= (matrix.at(bit) << bit % 8);
    if((bit + 1) % 8 == 0) {
      spi_data.push_back(std::make_pair(reg.address(), word));
      word = 0;
    }
  }

  LOG(DEBUG) << "Number of SPI commands: " << spi_data.size();

  // Finally, send the data over the SPI interface:
  _hal->send(spi_data);
}

void CLICpix2Device::configureClock(bool internal) {

  // Check of we should configure for external or internal clock, default to external:
  if(internal) {
    LOG(DEBUG) << "Configure internal clock source, free running, not locking";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers_free, SI5345_REVB_REG_CONFIG_NUM_REGS_FREE);
    mDelay(100); // let the PLL lock
  } else {
    LOG(DEBUG) << "Configure external clock source, locked to TLU input clock";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
    LOG(DEBUG) << "Waiting for clock to lock...";

    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock.");
    }
  }
}

void CLICpix2Device::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vdda") << "W";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(INFO) << "VDDCML:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vddcml") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vddcml") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vddcml") << "W";

  LOG(INFO) << "CMLBUFFERS_VCCO:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("cmlbuffers_vcco") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("cmlbuffers_vcco") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("cmlbuffers_vcco") << "W";

  LOG(INFO) << "CMLBUFFERS_VDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("cmlbuffers_vdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("cmlbuffers_vdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("cmlbuffers_vdd") << "W";
}

void CLICpix2Device::exploreInterface() {
  LOG(INFO) << "Exploring interface capabilities...";

  std::vector<std::pair<uint8_t, uint8_t>> pairvec;
  uint8_t defaultValues[] = {30,  50, 80,  90,  64, 136, 133, 133, 133, 133, 30, 50, 90, 0,
                             138, 0,  138, 133, 0,  0,   0,   0,   0,   0,   0,  88, 11};

  for(unsigned int i = 10; i < 63; i += 2)
    pairvec.push_back(std::make_pair(i, ~i));

  std::vector<std::pair<uint8_t, uint8_t>> default_rx = _hal->send(pairvec);
  for(unsigned int i = 0; i < default_rx.size(); i++)
    if(default_rx[i].second != defaultValues[i])
      throw DataCorrupt("Default register vaules mismatch");
  LOG(INFO) << "Success readout of default values of registers (addresses range 0x0a - 0x3E)";

  std::vector<std::pair<uint8_t, uint8_t>> rx = _hal->send(pairvec);
  for(unsigned int i = 0; i < rx.size(); i++)
    if(rx[i].second != pairvec[i].second)
      throw DataCorrupt("Data written at address " + to_hex_string(pairvec[i].first) + " doesn't match with read value");
  LOG(INFO) << "Sucess write-read back operation of regisers (addresses range 0x0a - 0x3E).";

  pairvec.clear();
  for(unsigned int i = 10, y = 0; i < 63; i += 2) {
    pairvec.push_back(std::make_pair(i, default_rx[y++].second));
  }
  _hal->send(pairvec);
  LOG(INFO) << "Reverting the default values of registers (addresses range 0x0a - 0x3E)";
  LOG(INFO) << "Exploring interface capabilities... Done";
}

void CLICpix2Device::daqStart() {
  // Prepare chip for data acquisition
}

pearydata CLICpix2Device::decodeFrame(const std::vector<uint32_t>& frame) {

  uint32_t comp = _register_cache["comp"];
  uint32_t sp_comp = _register_cache["sp_comp"];

  clicpix2_frameDecoder decoder((bool)comp, (bool)sp_comp, pixelsConfig);
  decoder.decode(frame);
  LOG(DEBUG) << "Decoded frame [row][column]:\n" << decoder;
  return decoder.getZerosuppressedFrame();
}

pearydata CLICpix2Device::getData() {
  return decodeFrame(getFrame());
}

std::vector<uint32_t> CLICpix2Device::getFrame() {

  LOG(DEBUG) << "Frame readout requested";
  this->setRegister("readout", 0);
  std::vector<uint32_t> frame;

  // Poll data until frameSize doesn't change anymore
  unsigned int frameSize = getMemory("frame_size");
  unsigned int frameSize_previous;
  do {
    frameSize_previous = frameSize;
    usleep(100);
    frameSize = getMemory("frame_size");
  } while(frameSize != frameSize_previous);

  frame.reserve(frameSize);
  for(unsigned int i = 0; i < frameSize; ++i) {
    frame.emplace_back(getMemory("frame"));
  }
  LOG(DEBUG) << "Read raw SerDes data:\n" << listVector(frame, ", ", true);
  return frame;
}

std::vector<uint32_t> CLICpix2Device::getRawData() {
  // Trigger the pattern generator to open the shutter and acquire one frame:
  triggerPatternGenerator(true);

  LOG(DEBUG) << "Preparing raw data packet";
  std::vector<uint32_t> rawdata;

  // Get the timestamps:
  auto timestamps = getTimestamps();

  // Read frame data:
  auto frame = getFrame();

  // Pack data: first data block is number of timestamp words, followed by timestamps and frame data
  rawdata.push_back(timestamps.size());
  rawdata.insert(rawdata.end(), timestamps.begin(), timestamps.end());
  rawdata.insert(rawdata.end(), frame.begin(), frame.end());

  LOG(DEBUG) << "Raw data packet with " << rawdata.size() << " words ready";
  return rawdata;
}

void CLICpix2Device::clearTimestamps() {
  // Retrieve all timestamps and throw them away:
  getTimestamps();
}

std::vector<uint32_t> CLICpix2Device::getTimestamps() {

  std::vector<uint32_t> timestamps;
  LOG(DEBUG) << "Requesting timestamps";

  // dummy readout
  if((getMemory("timestamp_msb") & 0x80000000) != 0) {
    LOG(WARNING) << "Timestamps FIFO is empty";
    return timestamps;
  }

  uint32_t ts_lsb;
  uint32_t ts_msb;
  do {
    // Read LSB
    ts_lsb = getMemory("timestamp_lsb");

    // Read MSB and remove top header bits
    ts_msb = getMemory("timestamp_msb");

    timestamps.push_back(ts_msb & 0x7ffff);
    timestamps.push_back(ts_lsb);
    LOG(DEBUG) << (ts_msb & 0x7ffff) << " | " << ts_lsb
               << "\t= " << ((static_cast<uint64_t>(ts_msb & 0x7ffff) << 32) | ts_lsb);
  } while(!(ts_msb & 0x80000000));
  LOG(DEBUG) << "Received " << timestamps.size() / 2 << " timestamps: " << listVector(timestamps, ",", false);

  return timestamps;
}

void CLICpix2Device::setOutputMultiplexer(std::string name) {
  std::map<std::string, int> output_mux_DAC{{"bias_disc_n", 1},
                                            {"bias_disc_p", 2},
                                            {"bias_thadj_dac", 3},
                                            {"bias_preamp_casc", 4},
                                            {"ikrum", 5},
                                            {"bias_preamp", 6},
                                            {"bias_buffers_1st", 7},
                                            {"bias_buffers_2st", 8},
                                            {"bias_thadj_casc", 9},
                                            {"bias_mirror_casc", 10},
                                            {"vfbk", 11},
                                            {"threshold", 12},
                                            {"threshold_lsb", 12},
                                            {"threshold_msb", 12},
                                            {"test_cap_2", 13},
                                            {"test_cap_1_lsb", 14},
                                            {"test_cap_1_msb", 14},
                                            {"test_cap_1", 14}};

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  this->setRegister("output_mux_DAC", output_mux_DAC[name]);
}
