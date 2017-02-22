/**
 * Caribou implementation for the C3PD
 */

#include "c3pd.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

C3PD::~C3PD() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

void C3PD::init() {
  LOG(logINFO) << DEVICE_NAME << ": Initialized.";
}

void C3PD::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up";
  LOG(logDEBUG) << " Reference voltage";
  voltageSet("c3pd_ref",C3PD_REF);
  LOG(logDEBUG) << " VDDD";
  voltageSet("c3pd_vddd",_config.Get("vddd",C3PD_VDDD));
  LOG(logDEBUG) << " VDDA";
  voltageSet("c3pd_vdda",_config.Get("vdda",C3PD_VDDD));
}

void C3PD::powerOff() {
  LOG(logINFO) << DEVICE_NAME << ": Power off.";
}

void C3PD::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void C3PD::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  C3PD* mDevice = new C3PD(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
