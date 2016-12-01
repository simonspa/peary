/**
 * Caribou HAL class implementation
 */

#include "hal.h"

using namespace caribou;

caribouHAL::caribouHAL() {};

caribouHAL::~caribouHAL() {};

std::vector<uint8_t> caribouHAL::sendCommandSPI(uint8_t address, std::vector<uint8_t> data) {

  // Cache the return values
  std::vector<uint8_t> miso_values;

  for(uint8_t d : data) {
    uint8_t retval = SPI(address, d);
      miso_values.push_back(retval);
  }

  return miso_values;
}

uint8_t caribouHAL::SPI(uint8_t address, uint8_t data) {

  // FIXME: Implement sending of the MOSI command
  // FIXME: Implement retrieval of MISO value
  return 0;
}

uint8_t getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}
