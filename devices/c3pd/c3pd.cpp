/**
 * Caribou implementation for the C3PD
 */

#include "c3pd.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

C3PD::~C3PD() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string C3PD::getName() {
  return DEVICE_NAME;
}

void C3PD::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up C3PD";

  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(PWR_OUT_2, _config.Get("vddd", C3PD_VDDD), _config.Get("vddd_current", C3PD_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_2, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(PWR_OUT_6, _config.Get("vdda", C3PD_VDDA), _config.Get("vdda_current", C3PD_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_6, true);

  // Fixme: Bias voltage below
  // LOG(logDEBUG) << " Reference voltage";
  // voltageSet("c3pd_ref",C3PD_REF);
}

void C3PD::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off C3PD";

  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator(PWR_OUT_6, false);

  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator(PWR_OUT_2, false);
}

void C3PD::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void C3PD::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

void C3PD::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_2) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_2) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_2) << "W";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_6) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_6) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_6) << "W";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  C3PD* mDevice = new C3PD(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
