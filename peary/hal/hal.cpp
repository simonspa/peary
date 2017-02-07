/**
 * Caribou HAL class implementation
 */

#include "exceptions.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "hal.hpp"

#include "interface_manager.hpp"
#include "interface.hpp"
#include "i2c.hpp"

using namespace caribou;

caribouHAL::caribouHAL(IFACE interface, std::string device_path = "") :
  _iface(interface), _devpath(device_path) {
  
  LOG(logDEBUGHAL) << "Configured device with typ-" << (int)_iface << " interface on " << _devpath;
}

caribouHAL::~caribouHAL() {}

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

std::vector<uint8_t>& caribouHAL::write(std::vector<uint8_t> address, std::vector<uint8_t> data) {

  Interface<uint8_t, uint8_t> & myi2c = interface_manager::getInterface<iface_i2c>("/dev/null");
}
