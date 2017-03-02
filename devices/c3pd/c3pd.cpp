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

void C3PD::init() {
  LOG(logINFO) << DEVICE_NAME << ": Initialized.";
}

void C3PD::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up C3PD";

  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator( PWR_OUT2,_config.Get("vddd",C3PD_VDDD) );
  _hal->powerVoltageRegulator( PWR_OUT2, true );

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator( PWR_OUT6,_config.Get("vdda",C3PD_VDDA) );
  _hal->powerVoltageRegulator( PWR_OUT6, true );


  //Fixme: Bias voltage below
  //LOG(logDEBUG) << " Reference voltage";
  //voltageSet("c3pd_ref",C3PD_REF);
}

void C3PD::powerOff() {
  LOG(logINFO) << DEVICE_NAME << ": Power off C3PD";
  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator( PWR_OUT2, false );

  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator( PWR_OUT6, false );

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
