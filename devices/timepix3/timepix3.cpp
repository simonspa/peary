/**
 * Caribou Device implementation for Timepix3
 */

#include "timepix3.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  timepix3* mDevice = new timepix3(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
