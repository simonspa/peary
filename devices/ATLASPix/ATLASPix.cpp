/**
 * Caribou implementation for the ATLASPix
 */


#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include "ATLASPix.hpp"
#include "hal.hpp"
#include "log.hpp"


using namespace caribou;

ATLASPix::ATLASPix(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), ATLASPix_DEFAULT_I2C) {

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_4);
  _periphery.add("vdda", PWR_OUT_3);
  _periphery.add("vssa", PWR_OUT_2);

  _periphery.add("CMOS_LEVEL", PWR_OUT_1);
	



  _periphery.add("GndDACPix_M2", BIAS_9);
  _periphery.add("VMinusPix_M2", BIAS_5);
  _periphery.add("GatePix_M2", BIAS_2);
  LOG(logINFO) << "Setting clock to 100MHz " << DEVICE_NAME;
  configureClock();


  // Add the register definitions to the dictionary for convenient lookup of names:
  //_registers.add(ATLASPix_REGISTERS);

  // Get access to FPGA memory mapped registers
  memfd = open(MEM_PATH, O_RDWR | O_SYNC);
  if(memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }

}

void ATLASPix::configure() {
  LOG(logINFO) << "Configuring " << DEVICE_NAME;
 

 volatile uint32_t* RAM_address = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS, 32, 0x0)));
 volatile uint32_t* RAM_write_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+1*4, 32, 0x0)));
 volatile uint32_t* RAM_content = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+2*4, 32, 0x0))); 
 volatile uint32_t* Config_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+3*4, 32, 0x0)));
 volatile uint32_t* RAM_reg_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+4*4, 32, 0x0)));
 volatile uint32_t* RAM_shift_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+5*4, 32, 0x0)));
 volatile uint32_t* global_reset = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+6*4, 32, 0x0))); 




 *RAM_reg_limit &= ~(0xFFFFFFFF);
 *RAM_shift_limit &= ~(0xFFFFFFFF);
 
  for(uint32_t i =0;i<64;i++){
	*RAM_address &= i;
	*RAM_content &= 0x0F;
	 usleep(10);
	*RAM_write_enable &=0xFFFFFFFF;
	 usleep(10);
	*RAM_write_enable &=0x0;};

 usleep(10);

 *Config_flag &= ~(0xFFFFFFFF);
 usleep(100);
 *Config_flag &= ~(0x0);
 LOG(logINFO) << "toggle of Load line" << DEVICE_NAME;
  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void ATLASPix::reset() {
  LOG(logDEBUG) << "Resetting " << DEVICE_NAME;
}

ATLASPix::~ATLASPix() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string ATLASPix::getName() {
  return DEVICE_NAME;
}

void ATLASPix::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up ATLASPix";

  // Power rails:
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(PWR_OUT_4, _config.Get("vddd", ATLASPix_VDDD), _config.Get("vddd_current", ATLASPix_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_4, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(PWR_OUT_3, _config.Get("vdda", ATLASPix_VDDA), _config.Get("vdda_current", ATLASPix_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_3, true);


  LOG(logDEBUG) << " VSSA";
  _hal->setVoltageRegulator(PWR_OUT_2, _config.Get("vssa", ATLASPix_VSSA), _config.Get("vssa_current", ATLASPix_VSSA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_2, true);

  LOG(logDEBUG) << " CMOS_Transcievers level";
  _hal->setVoltageRegulator(PWR_OUT_1, _config.Get("CMOS_LEVEL", ATLASPix_CMOS_LEVEL), _config.Get("cmos_level_current", ATLASPix_CMOS_LEVEL_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_1, true);



  // Bias voltages:
  LOG(logDEBUG) << " GNDDacPix ";
  _hal->setBiasRegulator(BIAS_9, _config.Get("GndDACPix_M2", ATLASPix_GndDACPix_M2));
  _hal->powerBiasRegulator(BIAS_9, true);

  LOG(logDEBUG) << " VMinusPix ";
  _hal->setBiasRegulator(BIAS_5, _config.Get("VMinusPix_M2", ATLASPix_VMinusPix_M2));
  _hal->powerBiasRegulator(BIAS_5, true);

  LOG(logDEBUG) << " GatePix_M2 ";
  _hal->setBiasRegulator(BIAS_2, _config.Get("GatePix_M2", ATLASPix_GatePix_M2));
  _hal->powerBiasRegulator(BIAS_2, true);

}

void ATLASPix::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off ATLASPix";

  LOG(logDEBUG) << "Powering off VDDA";
  _hal->powerVoltageRegulator(PWR_OUT_4, false);

  LOG(logDEBUG) << "Powering off VDDD";
  _hal->powerVoltageRegulator(PWR_OUT_3, false);

  LOG(logDEBUG) << "Powering off VSSA";
  _hal->powerVoltageRegulator(PWR_OUT_2, false);

  LOG(logDEBUG) << "Powering off CMOS_LEVEL";
  _hal->powerVoltageRegulator(PWR_OUT_1, false);

  LOG(logDEBUG) << "Turning off GNDDacPix";
  _hal->powerBiasRegulator(BIAS_9, true);

  LOG(logDEBUG) << "Turning off VMinusPix";
  _hal->powerBiasRegulator(BIAS_5, true);

  LOG(logDEBUG) << "Turning off GatePix_M2";
  _hal->powerBiasRegulator(BIAS_2, true);



}

void ATLASPix::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void ATLASPix::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

void ATLASPix::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_4) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_4) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_4) << "W";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_3) << "W";

  LOG(logINFO) << "VSSA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_2) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_2) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_2) << "W";

  LOG(logINFO) << "CMOS Level:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_1) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_1) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_1) << "W";

}




void ATLASPix::configureClock() {

  // Check of we should configure for external or internal clock, default to external:
  if(_config.Get<bool>("clock_internal", false)) {
    LOG(logDEBUG) << DEVICE_NAME << ": Configure internal clock source, free running, not locking";
    _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers_free, SI5345_REVB_REG_CONFIG_NUM_REGS_FREE);
    mDelay(100); // let the PLL lock
  } else {
    LOG(logDEBUG) << DEVICE_NAME << ": Configure external clock source, locked to TLU input clock";
    //_hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
    LOG(logDEBUG) << "Waiting for clock to lock...";

/*    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock."); 
    }*/
  }
}




caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  ATLASPix* mDevice = new ATLASPix(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
