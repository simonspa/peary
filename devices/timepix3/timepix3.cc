/**
 * Caribou Device implementation for Timepix3
 */

#include "timepix3.h"
#include "hal.h"
#include "log.h"

using namespace caribou;

caribouDevice* caribou::generator() {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  timepix3* mDevice = new timepix3();
  return dynamic_cast<caribouDevice*>(mDevice);
}
