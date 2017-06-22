/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include "hal.hpp"
#include "log.hpp"
#include "spi_CLICpix2.hpp"

#include "clicpix2_threshold.hpp"

using namespace caribou;

std::string clicpix2::getName() {
  return DEVICE_NAME;
}

clicpix2::clicpix2(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), 0), pg_total_length(0) {

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

  // Get access to FPGA memory mapped registers
  memfd = open(MEM_PATH, O_RDWR | O_SYNC);
  if(memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }

  // Map CLICpix2 receiver
  void* receiver_map_base;

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.
  receiver_map_base = mmap(0,
                           CLICPIX2_RECEIVER_MAP_SIZE,
                           PROT_READ,
                           MAP_SHARED,
                           memfd,
                           CLICPIX2_RECEIVER_BASE_ADDRESS & ~CLICPIX2_RECEIVER_MAP_MASK);
  if(receiver_map_base == (void*)-1) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page
  receiver_base = reinterpret_cast<void*>(reinterpret_cast<std::intptr_t>(receiver_map_base) +
                                          (CLICPIX2_RECEIVER_BASE_ADDRESS & CLICPIX2_RECEIVER_MAP_MASK));

  // Map CLICpix2 control
  void* control_map_base;

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.
  control_map_base = mmap(0,
                          CLICPIX2_CONTROL_MAP_SIZE,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          memfd,
                          CLICPIX2_CONTROL_BASE_ADDRESS & ~CLICPIX2_CONTROL_MAP_MASK);
  if(control_map_base == (void*)-1) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page
  control_base = reinterpret_cast<void*>(reinterpret_cast<std::intptr_t>(control_map_base) +
                                         (CLICPIX2_CONTROL_BASE_ADDRESS & CLICPIX2_CONTROL_MAP_MASK));

  // set default CLICpix2 control
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_RESET_OFFSET);
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
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_RESET_OFFSET);
  *control_reg &= ~(CLICPIX2_CONTROL_RESET_MASK); // assert reset
  usleep(1);
  *control_reg |= CLICPIX2_CONTROL_RESET_MASK; // deny reset
}

clicpix2::~clicpix2() {

  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();

  // Unamp CLICpix2 receiver
  if(munmap(receiver_base, CLICPIX2_RECEIVER_MAP_SIZE) == -1) {
    LOG(logCRITICAL) << "Can't unmap CLICpix2 receiver memory from user space.";
  }

  // Unamp CLICpix2 control
  if(munmap(control_base, CLICPIX2_CONTROL_MAP_SIZE) == -1) {
    LOG(logCRITICAL) << "Can't unmap CLICpix2 control memory from user space.";
  }

  close(memfd);
}

void clicpix2::setSpecialRegister(std::string name, uint32_t value) {

  if(name == "threshold") {
    // Linear threshold via lookup table

    // Get threshold LSB and MSB
    std::pair<uint8_t, uint8_t> dacs = threshold.at(value);
    LOG(logDEBUG) << "Threshold lookup: " << value << " = " << static_cast<int>(dacs.first) << "-"
                  << static_cast<int>(dacs.second);
    // Set the two values:
    this->setRegister("threshold_msb", dacs.first);
    this->setRegister("threshold_lsb", dacs.second);
  } else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

void clicpix2::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up CLICpix2";

  LOG(logDEBUG) << " CMLBUFFERS_VDD";
  _hal->setVoltageRegulator(PWR_OUT_4,
                            _config.Get("cmlbuffers_vdd", CLICpix2_CMLBUFFERS_VDD),
                            _config.Get("cmlbuffers_vdd_current", CLICpix2_CMLBUFFERS_VDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_4, true);

  LOG(logDEBUG) << " CMLBUFFERS_VCCO";
  _hal->setVoltageRegulator(PWR_OUT_7,
                            _config.Get("cmlbuffers_vcco", CLICpix2_CMLBUFFERS_VCCO),
                            _config.Get("cmlbuffers_vcco_current", CLICpix2_CMLBUFFERS_VCCO_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_7, true);

  LOG(logDEBUG) << " VDDCML";
  _hal->setVoltageRegulator(
    PWR_OUT_5, _config.Get("vdddcml", CLICpix2_VDDCML), _config.Get("vdddcml_current", CLICpix2_VDDCML_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_5, true);

  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(
    PWR_OUT_1, _config.Get("vddd", CLICpix2_VDDD), _config.Get("vddd_current", CLICpix2_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_1, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(
    PWR_OUT_3, _config.Get("vdda", CLICpix2_VDDA), _config.Get("vdda_current", CLICpix2_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_3, true);

  // FIXME:  _config.Get("cml_iref_pol",CLICpix2_CML_IREF_POL) ) doesn't compile
  LOG(logDEBUG) << " CML_IREF";
  _hal->setCurrentSource(CUR_1, _config.Get("cml_iref", CLICpix2_CML_IREF), CLICpix2_CML_IREF_POL);
  _hal->powerCurrentSource(CUR_1, true);

  // LOG(logDEBUG) << " DAC_IREF";
  // _hal->setCurrentSource( CUR_2, _config.Get("dac_iref",CLICpix2_DAC_IREF),
  // 			  CLICpix2_DAC_IREF_POL);
  // _hal->powerCurrentSource(CUR_2, true);
}

void clicpix2::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off CLICpix2";

  LOG(logDEBUG) << "Power off CML_IREF";
  _hal->powerCurrentSource(CUR_1, false);

  LOG(logDEBUG) << "Power off DAC_IREF";
  _hal->powerCurrentSource(CUR_2, false);

  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator(PWR_OUT_3, false);

  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator(PWR_OUT_1, false);

  LOG(logDEBUG) << "Power off VDDCML";
  _hal->powerVoltageRegulator(PWR_OUT_5, false);

  LOG(logDEBUG) << "Power off CMLBUFFERS_VCO";
  _hal->powerVoltageRegulator(PWR_OUT_7, false);

  LOG(logDEBUG) << "Power off CMLBUFFERS_VDD";
  _hal->powerVoltageRegulator(PWR_OUT_4, false);
}

void clicpix2::configureMatrix(std::string filename) {

  // Temporarily switch off any compression algorithm:
  uint32_t comp = this->getRegister("comp");
  uint32_t sp_comp = this->getRegister("sp_comp");
  this->setRegister("comp", 0);
  this->setRegister("sp_comp", 0);

  LOG(logDEBUG) << "Configuring the pixel matrix from file " << filename;
  readMatrix(filename);
  programMatrix();

  // Read back the matrix configuration and thus clear it:
  LOG(logDEBUG) << "Flushing matrix...";

  std::vector<uint32_t> frame = getRawData();
  clicpix2_frameDecoder decoder(false, false);

  try {
    decoder.decode(frame, pixelsConfig, false);

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
      }
    }
    LOG(logINFO) << "Verified matrix configuration.";
  } catch(caribou::DataException& e) {
    LOG(logERROR) << e.what();
  }

  // Reset compression state to previous values:
  this->setRegister("comp", comp);
  this->setRegister("sp_comp", sp_comp);
}

void clicpix2::readMatrix(std::string filename) {

  size_t masked = 0;
  LOG(logDEBUG) << "Reading pixel matrix file.";
  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    LOG(logERROR) << "Could not open matrix file \"" << filename << "\"";
    throw ConfigInvalid("Could not open matrix file \"" + filename + "\"");
  }

  std::string line = "";
  while(std::getline(pxfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    LOG(logDEBUGHAL) << "Read line: " << line;
    std::istringstream pxline(line);
    int column, row, threshold, mask, cntmode, tpenable, longcnt;
    if(pxline >> row >> column >> mask >> threshold >> cntmode >> tpenable >> longcnt) {
      pixelConfig px(mask, threshold, cntmode, tpenable, longcnt);
      pixelsConfig[std::make_pair(row, column)] = px;
      LOG(logDEBUGHAL) << "  is pixel: " << px;
      if(mask)
        masked++;
    }
  }
  LOG(logINFO) << "Now " << pixelsConfig.size() << " pixel configurations cached, " << masked << " of which are masked";
}

void clicpix2::triggerPatternGenerator(bool sleep) {

  LOG(logDEBUG) << "Triggering pattern generator once.";

  // Write into enable register of pattern generator:
  volatile uint32_t* wave_control =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + CLICPIX2_WAVE_CONTROL_OFFSET);

  // Toggle on:
  *wave_control &= ~(CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK);
  *wave_control |= CLICPIX2_CONTROL_WAVE_GENERATOR_ENABLE_MASK;

  // Wait for its lengcth before returning:
  if(sleep)
    usleep(pg_total_length / 10);
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
          LOG(logDEBUGHAL) << "bit " << bit << " of pixel " << row << "," << (2 * dcolumn + ((row + col) % 2));
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

  // Heavy debug output: print the full matrix bits
  IFLOG(logDEBUGHAL) {
    std::stringstream s;
    for(size_t bit = 0; bit < matrix.size(); bit++) {
      s << matrix.at(bit);
      if((bit + 1) % 8 == 0) {
        s << " ";
      }
      if((bit + 1) % 64 == 0)
        s << " (" << (bit + 1) / 64 << ")" << std::endl;
    }
    LOG(logDEBUGHAL) << "Full matrix bits:\n" << s.str();
  }

  LOG(logDEBUG) << "Number of SPI commands: " << spi_data.size();

  // Finally, send the data over the SPI interface:
  _hal->send(spi_data);
}

void clicpix2::configureClock() {
  LOG(logDEBUG) << DEVICE_NAME << ": Configure clock";
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
  while(!_hal->isLockedSI5345())
    ;
}

void clicpix2::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_3) << "W";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_1) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_1) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_1) << "W";

  LOG(logINFO) << "VDDACML:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_5) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_5) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_5) << "W";

  LOG(logINFO) << "CMLBUFFERS_VCO:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_7) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_7) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_7) << "W";

  LOG(logINFO) << "CMLBUFFERS_VDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_4) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_4) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_4) << "W";
}

void clicpix2::exploreInterface() {
  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities ...";

  std::vector<std::pair<uint8_t, uint8_t>> pairvec;
  uint8_t defualtValues[] = {30,  50, 80,  90,  64, 136, 133, 133, 133, 133, 30, 50, 90, 0,
                             138, 0,  138, 133, 0,  0,   0,   0,   0,   0,   0,  88, 11};

  for(unsigned int i = 10; i < 63; i += 2)
    pairvec.push_back(std::make_pair(i, ~i));

  std::vector<std::pair<uint8_t, uint8_t>> default_rx = _hal->send(pairvec);
  for(unsigned int i = 0; i < default_rx.size(); i++)
    if(default_rx[i].second != defualtValues[i])
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
  LOG(logINFO) << DEVICE_NAME << "Reverting the defualt values of registers (addresses range 0x0a - 0x3E)";

  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities ... Done";
}

void clicpix2::daqStart() {
  // Prepare chip for data acquisition
}

pearydata clicpix2::decodeFrame(const std::vector<uint32_t>& frame) {

  // FIXME not very efficient to always query the device...
  // Need a way to cache register values in the device!
  // Maybe via "special register" treatment?

  uint32_t comp = this->getRegister("comp");
  uint32_t sp_comp = this->getRegister("sp_comp");

  clicpix2_frameDecoder decoder((bool)comp, (bool)sp_comp);
  decoder.decode(frame, pixelsConfig);
  LOG(logDEBUG) << DEVICE_NAME << "Decoded frame [row][column]:\n" << decoder;
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

  LOG(logDEBUG) << DEVICE_NAME << " Read raw SerDes data:\n" << listVector(frame, ", ", true);
  return frame;
}
