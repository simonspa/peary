/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include "hal.hpp"
#include "log.hpp"
#include "spi_CLICpix2.hpp"

using namespace caribou;

std::string clicpix2::getName() {
  return DEVICE_NAME;
}

void clicpix2::init() {
  LOG(logDEBUG) << "Initializing " << DEVICE_NAME;
  configureClock();
}

clicpix2::~clicpix2() {

  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

void clicpix2::powerOn() {
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

void clicpix2::powerOff() {
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

  LOG(logDEBUG) << "Configuring the pixel matrix";
  readMatrix(filename);
  programMatrix();
  LOG(logDEBUG) << "...done!";
}

void clicpix2::readMatrix(std::string filename) {

  LOG(logDEBUG) << "Reading pixel matrix file.";
  std::ifstream pxfile(filename);
  std::string line = "";
  while(std::getline(pxfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pxline(line);
    int column, row, threshold, mask, cntmode, tpenable, longcnt;
    if(pxline >> column >> row >> mask >> threshold >> cntmode >> tpenable >> longcnt) {
      pixels[std::make_pair(column, row)] = pixelConfig(mask, threshold, cntmode, tpenable, longcnt);
    }
  }
  LOG(logDEBUG) << "Now " << pixels.size() << " pixel configurations cached.";
}

void clicpix2::programMatrix() {

  // Use a boolean vector to construct full matrix data array:
  std::vector<bool> matrix;

  // Loop over all (half-) rows, start with lowest:
  for(size_t row = 0; row < 256; row++) {
    // Store 14 bit per pixel:
    for(size_t bit = 0; bit < 14; bit++) {
      // Loop over all double columns
      for(size_t dcolumn = 0; dcolumn < 64; dcolumn++) {
        // Send one bit per double column to form one 64bit word
        pixelConfig px = pixels[std::make_pair(row / 2, 2 * dcolumn + row % 2)];
        matrix.push_back(px.GetBit(bit));
      }
    }
    LOG(logDEBUGAPI) << "One pixel done: " << row << ", odd: " << (row % 2) << " (matrix: " << matrix.size() << "b)";

    // After every superpixel (16 pixels), add one flip-flop per double column:
    if((row + 1) % 16 == 0) {
      matrix.insert(matrix.end(), 64, 0);
      LOG(logDEBUGAPI) << "Add superpixel flipflop for all double columns. (matrix: " << matrix.size() << "b)";
    }
  }

  // At the end of the column, add one flip-flop per double column:
  matrix.insert(matrix.end(), 64, 0);
  LOG(logDEBUG) << "Full matrix size: " << matrix.size() << "b";

  // At the very end, write one 64bit word with zeros to blank matrix after readout:
  matrix.insert(matrix.end(), 64, 0);
  LOG(logDEBUG) << "Full matrix size incl. clear: " << matrix.size() << "b";

  std::vector<std::pair<typename iface_spi_CLICpix2::reg_type, typename iface_spi_CLICpix2::data_type>> spi_data;
  register_t<> reg = _registers.get("matrix_programming");

  // Read matrix in 8b chunks to send over SPI interface:
  uint8_t word = 0;
  for(size_t bit = 0; bit < matrix.size(); bit++) {
    // Obey big-endianness of SPI: flip 8bit word endianness:
    word += (matrix.at(bit) << (7 - bit % 8));
    if((bit + 1) % 8 == 0) {
      spi_data.push_back(std::make_pair(reg.address(), word));
      word = 0;
    }
  }

  // Heavy debug output: print the full matrix bits
  IFLOG(logDEBUGHAL) {
    for(size_t bit = 0; bit < matrix.size(); bit++) {
      std::cout << matrix.at(bit);
      word += (matrix.at(bit) << (7 - bit % 8));
      if((bit + 1) % 8 == 0) {
        std::cout << "." << static_cast<int>(word) << " ";
        word = 0;
      }
      if((bit + 1) % 64 == 0)
        std::cout << " (" << (bit + 1) / 64 << ")" << std::endl;
    }
  }

  LOG(logDEBUG) << "Number of SPI commands: " << spi_data.size();
  // Finally, send the data over the SPI interface:
  //_hal->send(spi_data);
}

void clicpix2::configureClock() {
  LOG(logINFO) << DEVICE_NAME << ": Configure clock";
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
  // FIXME
  // while(! _hal-> isLockedSI5345() );
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

  unsigned int i = 10, y = 0;
  pairvec.clear();
  for(auto r : pairvec) {
    r.first = i;
    i += 2;
    r.second = default_rx[y++].second;
  }
  _hal->send(pairvec);
  LOG(logINFO) << DEVICE_NAME << "Reverting the defualt values of registers (addresses range 0x0a - 0x3E)";

  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities ... Done";
}

void clicpix2::daqStart() {
  const unsigned long CLICpix2_BASE_ADDRESS = 0x43C10000;
  const unsigned long CLICpix2_DATA_OFFSET = 0;
  const unsigned long MAP_SIZE = 4096;
  const unsigned long MAP_MASK = (MAP_SIZE - 1);

  int memfd;
  void *mapped_base, *mapped_dev_base;
  off_t dev_base = CLICpix2_BASE_ADDRESS;

  memfd = open("/dev/mem", O_RDWR | O_SYNC);
  if(memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }
  LOG(logINFO) << DEVICE_NAME << "/dev/mem opened";

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.

  mapped_base = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, memfd, dev_base & ~MAP_MASK);
  if(mapped_base == (void*)-1) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page

  //  mapped_dev_base = mapped_base + (dev_base & MAP_MASK);

  //  LOG(logINFO) << DEVICE_NAME <<   to_hex_string( *((volatile unsigned long *) (mapped_dev_base + CLICpix2_DATA_OFFSET))
  //  );

  // unmap the memory before exiting
  if(munmap(mapped_base, MAP_SIZE) == -1) {
    throw DeviceException("Can't unmap memory from user space.\n");
  }

  close(memfd);
  LOG(logINFO) << DEVICE_NAME << "/dev/mem closed";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  clicpix2* mDevice = new clicpix2(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
