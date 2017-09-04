/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include "clicpix2_utilities.hpp"

#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include "hal.hpp"
#include "log.hpp"
#include "spi_CLICpix2.hpp"

#include <math.h>

using namespace caribou;
using namespace clicpix2_utils;

std::string clicpix2::getName() {
  return DEVICE_NAME;
}

clicpix2::clicpix2(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH)), pg_total_length(0) {

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

  // set default CLICpix2 control
  volatile uint32_t* control_reg = reinterpret_cast<volatile uint32_t*>(
    reinterpret_cast<std::intptr_t>(
      _hal->getMappedMemoryRW(CLICPIX2_CONTROL_BASE_ADDRESS, CLICPIX2_CONTROL_MAP_SIZE, CLICPIX2_CONTROL_MAP_MASK)) +
    CLICPIX2_RESET_OFFSET);
  *control_reg = 0; // keep CLICpix2 in reset state
}

void clicpix2::configure() {
  LOG(logINFO) << "Configuring " << DEVICE_NAME;
  configureClock();
  reset();
  mDelay(10);

  // FIXME set all DACs provided with config
  // Call the base class configuration function:
  pearyDevice<iface_spi_CLICpix2>::configure();
}

void clicpix2::reset() {
  LOG(logDEBUG) << "Resetting " << DEVICE_NAME;
  volatile uint32_t* control_reg = reinterpret_cast<volatile uint32_t*>(
    reinterpret_cast<std::intptr_t>(
      _hal->getMappedMemoryRW(CLICPIX2_CONTROL_BASE_ADDRESS, CLICPIX2_CONTROL_MAP_SIZE, CLICPIX2_CONTROL_MAP_MASK)) +
    CLICPIX2_RESET_OFFSET);
  *control_reg &= ~(CLICPIX2_CONTROL_RESET_MASK); // assert reset
  usleep(1);
  *control_reg |= CLICPIX2_CONTROL_RESET_MASK; // deny reset
}

clicpix2::~clicpix2() {

  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

void clicpix2::setSpecialRegister(std::string name, uint32_t value) {

  if(name == "threshold") {
    // Get threshold LSB and MSB
    uint8_t msb = floor(value / 121.) * 14;
    uint8_t lsb = (value % 121) + 64;
    uint32_t maxThl = ceil(255 / 14.) * 121;
    if(value >= maxThl) {
      msb = 255;
      lsb = 255;
      LOG(logWARNING) << "Threshold range is limited to " << maxThl << ", setting 255-255";
    }
    LOG(logDEBUG) << "Threshold lookup: " << value << " = " << static_cast<int>(msb) << "-" << static_cast<int>(lsb);
    // Set the two values:
    this->setRegister("threshold_msb", msb);
    this->setRegister("threshold_lsb", lsb);
  } else if(name == "test_cap_1") {
    // Get pulsegen_counts LSB and MSB
    // std::pair<uint8_t,uint8_t> dacs = test_cap_1.at(value);
    LOG(logDEBUG) << "Test_cap_1 lookup: " << value << " = " << static_cast<int>((value >> 8) & 0x00FF) << "-"
                  << static_cast<int>(value & 0x00FF);
    // Set the two values:
    this->setRegister("test_cap_1_msb", (value >> 8) & 0x00FF);
    this->setRegister("test_cap_1_lsb", value & 0x00FF);
  } else if(name == "pulsegen_counts") {
    // Get pulsegen_counts LSB and MSB
    LOG(logDEBUG) << "Pulsegen_counts lookup: " << value << " = " << static_cast<int>((value >> 8) & 0x00FF) << "-"
                  << static_cast<int>(value & 0x00FF);
    // Set the two values:
    this->setRegister("pulsegen_counts_msb", (value >> 8) & 0x00FF);
    this->setRegister("pulsegen_counts_lsb", value & 0x00FF);
  } else if(name == "pulsegen_delay") {
    // Get pulsegen_counts LSB and MSB
    LOG(logDEBUG) << "Pulsegen_delay lookup: " << value << " = " << static_cast<int>((value >> 8) & 0x00FF) << "-"
                  << static_cast<int>(value & 0x00FF);
    // Set the two values:
    this->setRegister("pulsegen_delay_msb", (value >> 8) & 0x00FF);
    this->setRegister("pulsegen_delay_lsb", value & 0x00FF);
  } else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

void clicpix2::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up CLICpix2";

  LOG(logDEBUG) << " CMLBUFFERS_VDD";
  this->setVoltage("cmlbuffers_vdd",
                   _config.Get("cmlbuffers_vdd", CLICpix2_CMLBUFFERS_VDD),
                   _config.Get("cmlbuffers_vdd_current", CLICpix2_CMLBUFFERS_VDD_CURRENT));
  this->switchOn("cmlbuffers_vdd");

  LOG(logDEBUG) << " CMLBUFFERS_VCCO";
  this->setVoltage("cmlbuffers_vcco",
                   _config.Get("cmlbuffers_vcco", CLICpix2_CMLBUFFERS_VCCO),
                   _config.Get("cmlbuffers_vcco_current", CLICpix2_CMLBUFFERS_VCCO_CURRENT));
  this->switchOn("cmlbuffers_vcco");

  LOG(logDEBUG) << " VDDCML";
  this->setVoltage("vddcml", _config.Get("vddcml", CLICpix2_VDDCML), _config.Get("vddcml_current", CLICpix2_VDDCML_CURRENT));
  this->switchOn("vddcml");

  LOG(logDEBUG) << " VDDD";
  this->setVoltage("vddd", _config.Get("vddd", CLICpix2_VDDD), _config.Get("vddd_current", CLICpix2_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(logDEBUG) << " VDDA";
  this->setVoltage("vdda", _config.Get("vdda", CLICpix2_VDDA), _config.Get("vdda_current", CLICpix2_VDDA_CURRENT));
  this->switchOn("vdda");

  LOG(logDEBUG) << " CML_IREF";
  this->setCurrent("cml_iref", _config.Get("cml_iref", CLICpix2_CML_IREF), CLICpix2_CML_IREF_POL);
  this->switchOn("cml_iref");

  // Only enable if present in the configuration:
  if(_config.Has("dac_iref")) {
    LOG(logDEBUG) << " DAC_IREF";
    this->setCurrent("dac_iref", _config.Get("dac_iref", CLICpix2_DAC_IREF), CLICpix2_DAC_IREF_POL);
    this->switchOn("dac_iref");
  }
}

void clicpix2::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off CLICpix2";

  LOG(logDEBUG) << "Power off CML_IREF";
  this->switchOff("cml_iref");

  LOG(logDEBUG) << "Power off DAC_IREF";
  this->switchOff("dac_iref");

  LOG(logDEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(logDEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(logDEBUG) << "Power off VDDCML";
  this->switchOff("vddcml");

  LOG(logDEBUG) << "Power off CMLBUFFERS_VCCO";
  this->switchOff("cmlbuffers_vcco");

  LOG(logDEBUG) << "Power off CMLBUFFERS_VDD";
  this->switchOff("cmlbuffers_vdd");
}

void clicpix2::configureMatrix(std::string filename) {

  // Temporarily switch off any compression algorithm:
  uint32_t comp = this->getRegister("comp");
  uint32_t sp_comp = this->getRegister("sp_comp");
  this->setRegister("comp", 0);
  this->setRegister("sp_comp", 0);

  LOG(logDEBUG) << "Configuring the pixel matrix from file \"" << filename << "\"";
  pixelsConfig = readMatrix(filename);
  programMatrix();

  // Read back the matrix configuration and thus clear it:
  LOG(logDEBUG) << "Flushing matrix...";

  std::vector<uint32_t> frame = getRawData();
  clicpix2_frameDecoder decoder(false, false, pixelsConfig);

  try {
    decoder.decode(frame, false);
  } catch(caribou::DataException& e) {
    LOG(logERROR) << e.what();
    throw CommunicationError("Matrix configuration readout failed");
  }

  LOG(logINFO) << "Verifing matrix configuration...";

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
      LOG(logERROR) << "Matrix configuration of pixel " << static_cast<int>(px.first.second) << ","
                    << static_cast<int>(px.first.first) << " does not match:";
      LOG(logERROR) << to_bit_string(px.second.GetLatches()) << " != " << to_bit_string(pxv.GetLatches());
      configurationError = true;
    }
  }
  if(configurationError)
    throw CommunicationError("Matrix configuration failed");
  else
    LOG(logINFO) << "Verified matrix configuration.";

  // Reset compression state to previous values:
  this->setRegister("comp", comp);
  this->setRegister("sp_comp", sp_comp);
}

void clicpix2::triggerPatternGenerator(bool sleep) {

  LOG(logDEBUG) << "Triggering pattern generator once.";

  // Write into enable register of pattern generator:
  volatile uint32_t* wave_control = reinterpret_cast<volatile uint32_t*>(
    reinterpret_cast<std::intptr_t>(
      _hal->getMappedMemoryRW(CLICPIX2_CONTROL_BASE_ADDRESS, CLICPIX2_CONTROL_MAP_SIZE, CLICPIX2_CONTROL_MAP_MASK)) +
    CLICPIX2_WAVE_CONTROL_OFFSET);

  // Toggle on:
  *wave_control &= ~(CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK);
  *wave_control |= CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK;

  // Wait for its length before returning:
  if(sleep)
    usleep(pg_total_length / 100);
}

void clicpix2::configurePatternGenerator(std::string filename) {

  LOG(logDEBUG) << "Programming pattern generator.";
  std::vector<uint32_t> patterns;

  std::ifstream pgfile(filename);
  if(!pgfile.is_open()) {
    LOG(logERROR) << "Could not open pattern generator configuration file \"" << filename << "\"";
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
          LOG(logERROR) << "Unrecognized pattern for pattern generator: " << substr << " - ignoring.";
        }
      }
      pattern |= (duration & CLICPIX2_CONTROL_WAVE_GENERATOR_EVENTS_DURATION_MASK);
      LOG(logDEBUGHAL) << "PG signals: " << signals << " duration: " << duration << " pattern: " << to_bit_string(pattern);
      patterns.push_back(pattern);
      pg_total_length += duration;
    }
  }
  LOG(logDEBUG) << "Now " << patterns.size() << " patterns cached.";
  LOG(logDEBUG) << "Total length " << pg_total_length << " clk cycles.";
  if(patterns.size() > 32) {
    LOG(logCRITICAL) << "Pattern generator contains too many statements, maximum allowed: 32.";
    throw ConfigInvalid("Pattern generator too long");
  }

  void* control_base =
    _hal->getMappedMemoryRW(CLICPIX2_CONTROL_BASE_ADDRESS, CLICPIX2_CONTROL_MAP_SIZE, CLICPIX2_CONTROL_MAP_MASK);
  volatile uint32_t* wave_control =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_WAVE_CONTROL_OFFSET);

  // Switch loop mode off:
  *wave_control &= ~(CLICPIX2_CONTROL_WAVE_GENERATOR_LOOP_MODE_MASK);

  for(size_t reg = 0; reg < patterns.size(); reg++) {
    volatile uint32_t* wave_event = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) +
                                                                         CLICPIX2_WAVE_EVENTS_OFFSET + (reg << 2));
    *wave_event = patterns.at(reg);
  }

  LOG(logDEBUG) << "Done configuring pattern generator.";
}

void clicpix2::programMatrix() {

  // Use a boolean vector to construct full matrix data array:
  std::vector<bool> matrix;

  // At the end of the column, add one flip-flop per double column:
  LOG(logDEBUGAPI) << "Add EOC bits";
  matrix.insert(matrix.end(), 64, 0);

  // Loop over all rows, start with lowest:
  for(size_t row = 0; row < 128; row++) {

    // After every superpixel (16 pixels), add one flip-flop per double column:
    if(row % 8 == 0) {
      matrix.insert(matrix.end(), 64, 0);
      LOG(logDEBUGAPI) << "Add superpixel flipflop for all double columns. (matrix: " << matrix.size() << "b)";
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
      LOG(logDEBUGAPI) << "Pixel row " << row << ((row + col) % 2 ? " (odd)" : " (even)")
                       << " done, matrix size: " << matrix.size() << "bit";
    }
  }
  LOG(logDEBUG) << "Full matrix size: " << matrix.size() << "b";

  // At the very end, write one 64bit word with zeros to blank matrix after readout:
  matrix.insert(matrix.end(), 64, 0);
  LOG(logDEBUG) << "Full matrix size incl. clear: " << matrix.size() << "b";

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

  LOG(logDEBUG) << "Number of SPI commands: " << spi_data.size();

  // Finally, send the data over the SPI interface:
  _hal->send(spi_data);
}

void clicpix2::configureClock() {

  // Check of we should configure for external or internal clock, default to external:
  if(_config.Get<bool>("clock_internal", false)) {
    LOG(logDEBUG) << DEVICE_NAME << ": Configure internal clock source, free running, not locking";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers_free, SI5345_REVB_REG_CONFIG_NUM_REGS_FREE);
    mDelay(100); // let the PLL lock
  } else {
    LOG(logDEBUG) << DEVICE_NAME << ": Configure external clock source, locked to TLU input clock";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
    LOG(logDEBUG) << "Waiting for clock to lock...";

    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock.");
    }
  }
}

void clicpix2::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("vdda") << "W";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(logINFO) << "VDDCML:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("vddcml") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("vddcml") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("vddcml") << "W";

  LOG(logINFO) << "CMLBUFFERS_VCCO:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("cmlbuffers_vcco") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("cmlbuffers_vcco") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("cmlbuffers_vcco") << "W";

  LOG(logINFO) << "CMLBUFFERS_VDD:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("cmlbuffers_vdd") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("cmlbuffers_vdd") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("cmlbuffers_vdd") << "W";
}

void clicpix2::exploreInterface() {
  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities ...";

  std::vector<std::pair<uint8_t, uint8_t>> pairvec;
  uint8_t defaultValues[] = {30,  50, 80,  90,  64, 136, 133, 133, 133, 133, 30, 50, 90, 0,
                             138, 0,  138, 133, 0,  0,   0,   0,   0,   0,   0,  88, 11};

  for(unsigned int i = 10; i < 63; i += 2)
    pairvec.push_back(std::make_pair(i, ~i));

  std::vector<std::pair<uint8_t, uint8_t>> default_rx = _hal->send(pairvec);
  for(unsigned int i = 0; i < default_rx.size(); i++)
    if(default_rx[i].second != defaultValues[i])
      throw DataCorrupt("Default register vaules mismatch");
  LOG(logINFO) << DEVICE_NAME << "Success readout of default values of registers (addresses range 0x0a - 0x3E)";

  std::vector<std::pair<uint8_t, uint8_t>> rx = _hal->send(pairvec);
  for(unsigned int i = 0; i < rx.size(); i++)
    if(rx[i].second != pairvec[i].second)
      throw DataCorrupt("Data written at address " + to_hex_string(pairvec[i].first) + " doesn't match with read value");
  LOG(logINFO) << "Sucess write-read back operation of regisers (addresses range 0x0a - 0x3E).";

  pairvec.clear();
  for(unsigned int i = 10, y = 0; i < 63; i += 2) {
    pairvec.push_back(std::make_pair(i, default_rx[y++].second));
  }
  _hal->send(pairvec);
  LOG(logINFO) << DEVICE_NAME << "Reverting the default values of registers (addresses range 0x0a - 0x3E)";

  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities ... Done";
}

void clicpix2::daqStart() {
  // Prepare chip for data acquisition
}

pearydata clicpix2::decodeFrame(const std::vector<uint32_t>& frame) {

  uint32_t comp = _register_cache["comp"];
  uint32_t sp_comp = _register_cache["sp_comp"];

  clicpix2_frameDecoder decoder((bool)comp, (bool)sp_comp, pixelsConfig);
  decoder.decode(frame);
  LOG(logDEBUGAPI) << DEVICE_NAME << "Decoded frame [row][column]:\n" << decoder;
  return decoder.getZerosuppressedFrame();
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  clicpix2* mDevice = new clicpix2(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}

pearydata clicpix2::getData() {
  return decodeFrame(getRawData());
}

std::vector<uint32_t> clicpix2::getRawData() {

  LOG(logDEBUG) << DEVICE_NAME << " readout requested";
  this->setRegister("readout", 0);
  std::vector<uint32_t> frame;

  void* receiver_base =
    _hal->getMappedMemoryRO(CLICPIX2_RECEIVER_BASE_ADDRESS, CLICPIX2_RECEIVER_MAP_SIZE, CLICPIX2_RECEIVER_MAP_MASK);

  // Poll data until frameSize doesn't change anymore
  unsigned int frameSize, frameSize_previous;
  frameSize = *(
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(receiver_base) + CLICPIX2_RECEIVER_COUNTER_OFFSET));
  do {
    frameSize_previous = frameSize;
    usleep(100);
    frameSize = *(reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(receiver_base) +
                                                       CLICPIX2_RECEIVER_COUNTER_OFFSET));

  } while(frameSize != frameSize_previous);

  frame.reserve(frameSize);
  for(unsigned int i = 0; i < frameSize; ++i) {
    frame.emplace_back(*(
      reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(receiver_base) + CLICPIX2_RECEIVER_FIFO_OFFSET)));
  }

  LOG(logDEBUGAPI) << DEVICE_NAME << " Read raw SerDes data:\n" << listVector(frame, ", ", true);
  return frame;
}

std::vector<uint64_t> clicpix2::timestampsPatternGenerator() {

  std::vector<uint64_t> timestamps;
  LOG(logDEBUG) << DEVICE_NAME << " Requesting timestamps";

  void* control_base =
    _hal->getMappedMemoryRW(CLICPIX2_CONTROL_BASE_ADDRESS, CLICPIX2_CONTROL_MAP_SIZE, CLICPIX2_CONTROL_MAP_MASK);
  // Write into enable register of pattern generator:
  volatile uint32_t* timestamp_lsb =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_TIMESTAMPS_LSB_OFFSET);
  volatile uint32_t* timestamp_msb =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_TIMESTAMPS_MSB_OFFSET);

  uint64_t timestamp;

  // dummy readout
  if((*timestamp_msb & 0x80000000) != 0) {
    LOG(logWARNING) << DEVICE_NAME << " Timestamps FIFO is empty: ";
    return timestamps;
  }

  do {
    timestamp = *timestamp_lsb;
    timestamp |= (static_cast<uint64_t>(*timestamp_msb) << 32);
    timestamps.push_back(timestamp & 0x7ffffffffffff);
  } while(!(timestamp & 0x8000000000000000));
  LOG(logDEBUG) << DEVICE_NAME << " Received timestamps: " << listVector(timestamps, ",", false);

  return timestamps;
}
