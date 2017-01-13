/**
 * Caribou HAL class implementation
 */

#include "log.h"
#include "hal.h"

#include "spi.h"
#include "i2c.h"

using namespace caribou;

caribouHAL::caribouHAL(uint8_t interface) {

  LOG(logDEBUGHAL) << "Configured device requires typ-" << (int)interface << " interface.";
};

caribouHAL::~caribouHAL() {};

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

