/**
 * Caribou implementation for the C3PD
 */

#include "c3pd.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

C3PD::C3PD(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), C3PD_DEFAULT_I2C) {

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);
  _periphery.add("vdda", PWR_OUT_6);

  _periphery.add("ain", BIAS_1);
  _periphery.add("ref", BIAS_2);

  _periphery.add("aout", VOL_IN_2);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(C3PD_REGISTERS);

  // Get access to FPGA memory mapped registers
  memfd = open(MEM_PATH, O_RDWR | O_SYNC);
  if(memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }

  // Map C3PD control
  void* control_map_base;

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.
  control_map_base = mmap(
    0, C3PD_CONTROL_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, C3PD_CONTROL_BASE_ADDRESS & ~C3PD_CONTROL_MAP_MASK);
  if(control_map_base == (void*)-1) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page
  control_base = reinterpret_cast<void*>(reinterpret_cast<std::intptr_t>(control_map_base) +
                                         (C3PD_CONTROL_BASE_ADDRESS & C3PD_CONTROL_MAP_MASK));

  // set default C3PD control
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + C3PD_RESET_OFFSET);
  *control_reg = 0; // keep C3PD in reset state
}

void C3PD::configure() {
  LOG(logINFO) << "Configuring " << DEVICE_NAME;
  reset();
}

void C3PD::reset() {
  LOG(logDEBUG) << "Reseting " << DEVICE_NAME;
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + C3PD_RESET_OFFSET);
  *control_reg &= ~(C3PD_CONTROL_RESET_MASK); // assert reset
  usleep(1);
  *control_reg |= C3PD_CONTROL_RESET_MASK; // deny reset
}

C3PD::~C3PD() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string C3PD::getName() {
  return DEVICE_NAME;
}

void C3PD::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up C3PD";

  // Power rails:
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(PWR_OUT_2, _config.Get("vddd", C3PD_VDDD), _config.Get("vddd_current", C3PD_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_2, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(PWR_OUT_6, _config.Get("vdda", C3PD_VDDA), _config.Get("vdda_current", C3PD_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_6, true);

  // Bias voltages:
  LOG(logDEBUG) << " Reference voltage";
  _hal->setBiasRegulator(BIAS_2, _config.Get("vdda", C3PD_REF));
  _hal->powerBiasRegulator(BIAS_2, true);

  LOG(logDEBUG) << " Analog-In";
  _hal->setBiasRegulator(BIAS_1, _config.Get("ain", C3PD_AIN));
  _hal->powerBiasRegulator(BIAS_1, true);
}

void C3PD::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off C3PD";

  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator(PWR_OUT_6, false);

  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator(PWR_OUT_2, false);

  LOG(logDEBUG) << "Turn off AIN";
  _hal->powerBiasRegulator(BIAS_1, true);

  LOG(logDEBUG) << "Turn off REF";
  _hal->powerBiasRegulator(BIAS_2, true);
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
