/**
 * Caribou implementation for the ATLASPix
 */


#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>
#include <string>
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

  this->Initialize_SR();


  // Add the register definitions to the dictionary for convenient lookup of names:
  //_registers.add(ATLASPix_REGISTERS);

   //Get access to FPGA memory mapped registers
//  memfd = open(MEM_PATH, O_RDWR | O_SYNC);
//  if(memfd == -1) {
//    throw DeviceException("Can't open /dev/mem.\n");
//  }




}

void ATLASPix::configure() {
 LOG(logINFO) << "Configuring " << DEVICE_NAME;
 
 // Build the SR string with default values and shift in the values in the chip
  this->Fill_SR();
  this->Shift_SR();

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


void ATLASPix::Initialize_SR(){


    CurrentDACConfig = new ATLASPix_Config();
    MatrixDACConfig = new ATLASPix_Config();
    VoltageDACConfig = new ATLASPix_Config();


    //DAC Block 1 for DIgital Part
    //AnalogDACs
    CurrentDACConfig->AddParameter("unlock",    4, ATLASPix_Config::LSBFirst, 0); // unlock = x101
    CurrentDACConfig->AddParameter("BLResPix", "5,4,3,1,0,2",  5);
    CurrentDACConfig->AddParameter("ThResPix", "5,4,3,1,0,2",  0);
    CurrentDACConfig->AddParameter("VNPix", "5,4,3,1,0,2",  20);
    CurrentDACConfig->AddParameter("VNFBPix", "5,4,3,1,0,2", 10);
    CurrentDACConfig->AddParameter("VNFollPix", "5,4,3,1,0,2", 10);
    CurrentDACConfig->AddParameter("VNRegCasc", "5,4,3,1,0,2", 20);     //hier : VNHitbus
    CurrentDACConfig->AddParameter("VDel", "5,4,3,1,0,2", 10);
    CurrentDACConfig->AddParameter("VPComp", "5,4,3,1,0,2", 20);        //hier : VPHitbus
    CurrentDACConfig->AddParameter("VPDAC", "5,4,3,1,0,2",  0);
    CurrentDACConfig->AddParameter("VNPix2", "5,4,3,1,0,2",  0);
    CurrentDACConfig->AddParameter("BLResDig", "5,4,3,1,0,2",  5);
    CurrentDACConfig->AddParameter("VNBiasPix", "5,4,3,1,0,2",  0);
    CurrentDACConfig->AddParameter("VPLoadPix", "5,4,3,1,0,2",  5);
    CurrentDACConfig->AddParameter("VNOutPix", "5,4,3,1,0,2", 5);
    //DigitalDACs
    CurrentDACConfig->AddParameter("VPVCO", "5,4,3,1,0,2",  7);//5);//7);
    CurrentDACConfig->AddParameter("VNVCO", "5,4,3,1,0,2",  15);//15);
    CurrentDACConfig->AddParameter("VPDelDclMux", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VNDelDclMux", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VPDelDcl", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VNDelDcl", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VPDelPreEmp", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VNDelPreEmp", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VPDcl", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VNDcl", "5,4,3,1,0,2",  30);//30);
    CurrentDACConfig->AddParameter("VNLVDS", "5,4,3,1,0,2",  10);//10);
    CurrentDACConfig->AddParameter("VNLVDSDel", "5,4,3,1,0,2",  00);//10);
    CurrentDACConfig->AddParameter("VPPump", "5,4,3,1,0,2",  5);//5);

    CurrentDACConfig->AddParameter("nu", "1,0",  0);
    CurrentDACConfig->AddParameter("RO_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
    CurrentDACConfig->AddParameter("Ser_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
    CurrentDACConfig->AddParameter("Aur_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
    CurrentDACConfig->AddParameter("sendcnt",     1, ATLASPix_Config::LSBFirst,  0);//0);
    CurrentDACConfig->AddParameter("resetckdivend", "3,2,1,0",  0);//2);
    CurrentDACConfig->AddParameter("maxcycend", "5,4,3,2,1,0",  63);//10); // probably 0 not allowed
    CurrentDACConfig->AddParameter("slowdownend", "3,2,1,0",  0);//1);
    CurrentDACConfig->AddParameter("timerend", "3,2,1,0",  1);//8); // darf nicht 0!! sonst werden debug ausgaben verschluckt
    CurrentDACConfig->AddParameter("ckdivend2", "5,4,3,2,1,0",  0);//1);
    CurrentDACConfig->AddParameter("ckdivend", "5,4,3,2,1,0",  0);//1);
    CurrentDACConfig->AddParameter("VPRegCasc", "5,4,3,1,0,2",  20);
    CurrentDACConfig->AddParameter("VPRamp", "5,4,3,1,0,2",  0); // was 4, off for HB/Thlow usage and fastreadout
    CurrentDACConfig->AddParameter("VNcompPix", "5,4,3,1,0,2",  0);     //VNComparator
    CurrentDACConfig->AddParameter("VPFoll", "5,4,3,1,0,2",  10);
    CurrentDACConfig->AddParameter("VNDACPix", "5,4,3,1,0,2",  0);
    CurrentDACConfig->AddParameter("VPBiasRec", "5,4,3,1,0,2",  30);
    CurrentDACConfig->AddParameter("VNBiasRec", "5,4,3,1,0,2",  30);
    CurrentDACConfig->AddParameter("Invert",     1, ATLASPix_Config::LSBFirst, 0);// 0);
    CurrentDACConfig->AddParameter("SelEx",     1, ATLASPix_Config::LSBFirst,  1);//1); //activated external clock input
    CurrentDACConfig->AddParameter("SelSlow",     1, ATLASPix_Config::LSBFirst,  1);//1);
    CurrentDACConfig->AddParameter("EnPLL",     1, ATLASPix_Config::LSBFirst,  0);//0);
    CurrentDACConfig->AddParameter("TriggerDelay",     10, ATLASPix_Config::LSBFirst,  0);
    CurrentDACConfig->AddParameter("Reset", 1, ATLASPix_Config::LSBFirst, 0);
    CurrentDACConfig->AddParameter("ConnRes",     1, ATLASPix_Config::LSBFirst,  1);//1);   //activates termination for output lvds
    CurrentDACConfig->AddParameter("SelTest",     1, ATLASPix_Config::LSBFirst,  0);
    CurrentDACConfig->AddParameter("SelTestOut",     1, ATLASPix_Config::LSBFirst,  0);



    //Column Register
    for (int col = 0; col <28; col++)
    {

    	std::string s = to_string(col);
    	MatrixDACConfig->AddParameter("RamL"+s, 3, ATLASPix_Config::LSBFirst,  0);
    	MatrixDACConfig->AddParameter("colinjL"+s, 1, ATLASPix_Config::LSBFirst,  0);
    	MatrixDACConfig->AddParameter("RamR"+s, 3, ATLASPix_Config::LSBFirst,  0);
    	MatrixDACConfig->AddParameter("colinjR"+s, 1, ATLASPix_Config::LSBFirst,  0);

    }


    //Row Register
    for (int row = 0; row < 320; row++)
    {
    	std::string s = to_string(row);
    	MatrixDACConfig->AddParameter("writedac"+s, 1, ATLASPix_Config::LSBFirst, 0);
    	MatrixDACConfig->AddParameter("unused"+s,   3, ATLASPix_Config::LSBFirst, 0);
    	MatrixDACConfig->AddParameter("rowinjection"+s, 1, ATLASPix_Config::LSBFirst, 0);
    	MatrixDACConfig->AddParameter("analogbuffer"+s, 1, ATLASPix_Config::LSBFirst, 0);
    }



    VoltageDACConfig->AddParameter("BLPix", 8,ATLASPix_Config::LSBFirst, floor(255 * this->BLPix/1.8));
    VoltageDACConfig->AddParameter("nu2", 2, ATLASPix_Config::LSBFirst, this->nu2);
    VoltageDACConfig->AddParameter("ThPix", 8, ATLASPix_Config::LSBFirst, floor(255 * this->ThPix/1.8));
    VoltageDACConfig->AddParameter("nu3", 2, ATLASPix_Config::LSBFirst, this->nu3);
}


void ATLASPix::Fill_SR()
{
  
    CurrentDACbits = CurrentDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
    MatrixBits = MatrixDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
    VoltageDACBits = VoltageDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);

    uint32_t buffer =0;
    uint32_t cnt =0;

    this->Registers.clear();

    for (auto i = CurrentDACbits.begin(); i != CurrentDACbits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 this->Registers.push_back(buffer);
	 std::cout << buffer << " ";
	 this->printBits(sizeof(buffer),&buffer);
	 std::cout <<  std::endl;	 
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;   
     }
   for (auto i = MatrixBits.begin(); i != MatrixBits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 this->Registers.push_back(buffer);
	 std::cout << buffer << " ";
	 this->printBits(sizeof(buffer),&buffer);
	 std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;   
     }
   for (auto i = VoltageDACBits.begin(); i != VoltageDACBits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 this->Registers.push_back(buffer);
	 std::cout << buffer << " ";
	 this->printBits(sizeof(buffer),&buffer);
	 std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;   
     }
     std::cout << buffer << " ";
     this->printBits(sizeof(buffer),&buffer);
     std::cout <<  std::endl;
}

void ATLASPix::Shift_SR(){

 LOG(logINFO) << "mapping RAM registers " << DEVICE_NAME;
 volatile uint32_t* RAM_address = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS, 32, 0x0)));
 volatile uint32_t* RAM_write_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+1*4, 32, 0x0)));
 volatile uint32_t* RAM_content = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+2*4, 32, 0x0))); 
 LOG(logINFO) << "mapping Config registers " << DEVICE_NAME;
 volatile uint32_t* Config_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+3*4, 32, 0x0)));
 volatile uint32_t* RAM_reg_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+4*4, 32, 0x0)));
 volatile uint32_t* RAM_shift_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+5*4, 32, 0x0)));
 //volatile uint32_t* global_reset = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(_hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS+6*4, 32, 0x0)));




 *RAM_reg_limit &= ~(0xFFFFFFFF);
 *RAM_shift_limit &= ~(0xFFFFFFFF);
 
  for(uint32_t i =0;i<64;i++){
	*RAM_address &= i;
	*RAM_content &= Registers(i);
	 usleep(10);
	*RAM_write_enable &=0xFFFFFFFF;
	 usleep(10);
	*RAM_write_enable &=0x0;};

 usleep(10);

 *Config_flag &= (0xFFFFFFFF);
 usleep(100);
 *Config_flag &= (0x0);
}


void ATLASPix::printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  ATLASPix* mDevice = new ATLASPix(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
