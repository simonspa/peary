/**
 * Caribou implementation for the ATLASPix
 */


#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>
#include <string>
#include "ATLASPix.hpp"
#include "hal.hpp"
#include "log.hpp"
#include <iostream>
#include <cstdarg>
#include <ctime>
#include <stdexcept>
#include <iomanip>

using namespace caribou;


/*
uint32_t reverseBits(uint32_t n) {
        uint32_t x;
        for(auto i = 31; n; ) {
            x |= (n & 1) << i;
            n >>= 1;
            -- i;
        }
        return x;
    }
*/

// BASIC Configuration


ATLASPix::ATLASPix(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), ATLASPix_DEFAULT_I2C) {

  // Set up periphery
  _periphery.add("VDDD", PWR_OUT_4);
  _periphery.add("VDDA", PWR_OUT_3);
  _periphery.add("VSSA", PWR_OUT_2);
  //_periphery.add("CMOS_LEVEL", PWR_OUT_1);
  _periphery.add("GndDACPix_M1", BIAS_9);
  _periphery.add("VMinusPix_M1", BIAS_5);
  _periphery.add("GatePix_M1", BIAS_2);

  _periphery.add("GndDACPix_M2", BIAS_6);
  _periphery.add("VMinusPix_M2", BIAS_4);
  _periphery.add("GatePix_M2", BIAS_1);

  _periphery.add("GndDACPix_M1ISO", BIAS_12);
  _periphery.add("VMinusPix_M1ISO", BIAS_8);
  _periphery.add("GatePix_M1ISO", BIAS_3);

  //Data structure containing the info about the matrices
  this->simpleM1 = new ATLASPixMatrix();
  this->simpleM1ISO = new ATLASPixMatrix();
  this->simpleM2 = new ATLASPixMatrix();

  this->simpleM2->BLPix=0.8-0.036;
  this->simpleM2->ThPix=0.86+0.014;
  this->simpleM1->BLPix=0.8-0.036;
  this->simpleM1->ThPix=0.86+0.014;
  this->simpleM1ISO->BLPix=0.8-0.036;
  this->simpleM1ISO->ThPix=0.86+0.014;


  this->simpleM1->ncol=ncol_m1;
  this->simpleM1ISO->ncol=ncol_m1iso;
  this->simpleM2->ncol=ncol_m2;

  this->simpleM1->ndoublecol=ncol_m1/2;
  this->simpleM1ISO->ndoublecol=ncol_m1iso/2;
  this->simpleM2->ndoublecol=ncol_m2/2;

  this->simpleM1->nrow=nrow_m1;
  this->simpleM1ISO->nrow=nrow_m1iso;
  this->simpleM2->nrow=nrow_m2;

  this->simpleM1->counter=2;
  this->simpleM1ISO->counter=1;
  this->simpleM2->counter=3;

  this->simpleM1->nSRbuffer = 104;
  this->simpleM1ISO->nSRbuffer = 104;
  this->simpleM2->nSRbuffer = 84;

  this->simpleM1->extraBits = 16;
  this->simpleM1ISO->extraBits = 16;
  this->simpleM2->extraBits = 0;

  this->simpleM1->SRmask=0x2;
  this->simpleM1ISO->SRmask=0x4;
  this->simpleM2->SRmask=0x1;

  this->simpleM1->PulserMask=0x2;
  this->simpleM2->PulserMask=0x1;
  this->simpleM1ISO->PulserMask=0x4;



  this->simpleM1->type=1;
  this->simpleM1ISO->type=1;
  this->simpleM2->type=2;


  //Configuring the clock to 160 MHz
  LOG(logINFO) << "Setting clock to 160MHz " << DEVICE_NAME;
  configureClock();

  _registers.add(ATLASPix_REGISTERS);



  this->Initialize_SR(this->simpleM1);
  this->Initialize_SR(this->simpleM1ISO);
  this->Initialize_SR(this->simpleM2);





  void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
  volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);
  volatile uint32_t* pulse_count = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x4);
  volatile uint32_t* high_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x8);
  volatile uint32_t* low_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0xC);
  volatile uint32_t* output_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x10);
  volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);

  *inj_flag = 0x0;
  *pulse_count = 0x0;
  *high_cnt = 0x0;
  *low_cnt = 0x0;
  *output_enable = 0xFFFFFFFF;
  *rst = 0x1;

}

void ATLASPix::configure() {
 LOG(logINFO) << "Configuring " << DEVICE_NAME;

 this->resetPulser();
 this->resetCounters();

 //this->powerOn();
 usleep(1000);


 // Build the SR string with default values and shift in the values in the chip
 	std::cout << "sending default config " << std::endl;
 	this->ProgramSR(simpleM1ISO);
	this->ProgramSR(simpleM1);
	this->ProgramSR(simpleM2);
	//sleep(10);
	//this->ProgramSR(simpleM1);
	//sleep(10);
	//this->ProgramSR(simpleM1ISO);
	//sleep(10);

  //this->ComputeSCurves(0,0.5,50,128,100,100);
 std::cout << "sending default TDACs " << std::endl;

  this->writeUniformTDAC(simpleM1,0b0000);
  this->writeUniformTDAC(simpleM1ISO,0b0000);
  this->writeUniformTDAC(simpleM2,0b000);

  std::cout << "loading TDACs" << std::endl;
  //this->loadAllTDAC("/home/root/TDAC.txt");
  std::cout << "done" << std::endl;



//  this->resetCounters();
//  this->setPulse(simpleM1,1,1000000,1000000, 0.5);
//
//
//  while(1){
//	  std::cout << "sending pulse" << std::endl;
//	  this->sendPulse();
//	  usleep(2000);
//	  //std::cout << "Counter 0 : " << this->readCounter(simpleM1) << std::endl;
//	  //std::cout << "Counter 1 : " << this->readCounter(simpleM1ISO) << std::endl;
//	  //std::cout << "Counter 2 : " << this->readCounter(simpleM2) << std::endl;
//	  int ddd = -1;
//	  std::cin >> ddd;
//	 if (ddd==0){break;}
//  }



 // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void ATLASPix::lock(){

	this->simpleM2->CurrentDACConfig->SetParameter("unlock",0x0);
	this->ProgramSR(simpleM2);

	this->simpleM1->CurrentDACConfig->SetParameter("unlock",0x0);
	this->ProgramSR(simpleM1);

	this->simpleM1ISO->CurrentDACConfig->SetParameter("unlock",0x0);
	this->ProgramSR(simpleM1ISO);
}

void ATLASPix::unlock(){


	this->simpleM2->CurrentDACConfig->SetParameter("unlock",0b1010);
	this->ProgramSR(simpleM2);


	this->simpleM1->CurrentDACConfig->SetParameter("unlock",0b1010);
	this->ProgramSR(simpleM1);


	this->simpleM1ISO->CurrentDACConfig->SetParameter("unlock",0b1010);
	this->ProgramSR(simpleM1ISO);



}

void ATLASPix::setThreshold(double threshold){

	simpleM2->VoltageDACConfig->SetParameter("ThPix",static_cast<int>(floor(255 * threshold/1.8)));
	simpleM1->VoltageDACConfig->SetParameter("ThPix",static_cast<int>(floor(255 * threshold/1.8)));
	simpleM1ISO->VoltageDACConfig->SetParameter("ThPix",static_cast<int>(floor(255 * threshold/1.8)));

	this->ProgramSR(simpleM2);
	this->ProgramSR(simpleM1);
	this->ProgramSR(simpleM1ISO);

	  LOG(logDEBUG) << " ThPix m1 ISO ";
	  _hal->setBiasRegulator(BIAS_31, _config.Get("ThPix_M1ISO", threshold));
	  _hal->powerBiasRegulator(BIAS_31, true);


	  LOG(logDEBUG) << " ThPix m1  ";
	  _hal->setBiasRegulator(BIAS_25, _config.Get("ThPix_M1", threshold));
	  _hal->powerBiasRegulator(BIAS_25, true);


	  LOG(logDEBUG) << " ThPix m2 ";
	  _hal->setBiasRegulator(BIAS_28, _config.Get("ThPix_M2", threshold));
	  _hal->powerBiasRegulator(BIAS_28, true);

	simpleM1->ThPix=threshold;
	simpleM2->ThPix=threshold;
	simpleM1ISO->ThPix=threshold;

}

void ATLASPix::setSpecialRegister(std::string name, uint32_t value) {

		std::cout << '\n' << "***You have set " << name << " as " << std::dec << value <<  "***" << '\n' << '\n';

		char Choice = '1' ;

		if(name == "unlock") {
	    // Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("unlock",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					this->simpleM2->CurrentDACConfig->SetParameter("unlock",0x0);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("unlock",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
	    }

		else if (name == "blrespix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "threspix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}

		else if (name == "vnpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;
			Choice = '1' ;
			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}

		else if (name == "vnfbpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;
			Choice='1';
			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnfollpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnregcasc") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vdel") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpcomp") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdac") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnpix2") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "blresdig") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnbiaspix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vploadpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnoutpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpvco") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnvco") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdeldclmux") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndeldclmux") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdeldcl") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndeldcl") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdelpreemp") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndelpreemp") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdcl") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndcl") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnlvds") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnlvdsdel") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vppump") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "nu") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ro_res_n") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ser_res_n") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "aur_res_n") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "sendcnt") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "resetckdivend") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "maxcycend") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "slowdownend") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "timerend") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ckdivend2") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ckdivend") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpregcasc") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpramp") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vncomppix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpfoll") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndacpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpbiasrec") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnbiasrec") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "invert") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "selex") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "selslow") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "enpll") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "triggerdelay") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "reset") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "connres") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "seltest") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "seltestout") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "blpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "nu2") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "thpix") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
		else if (name == "nu3") {
		// Set DAC value here calling setParameter
			std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					simpleM1->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(simpleM1);
					break ;
				}
			case '2' :
				{
					simpleM2->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(simpleM2);
					break ;
				}
			case '3' :
				{
					simpleM1ISO->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(simpleM1ISO);
					break ;
				}
			}
		}
	else {
	    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
	}
}


void ATLASPix::configureClock() {
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
}


void ATLASPix::Initialize_SR(ATLASPixMatrix *matrix){

	matrix->CurrentDACConfig = new ATLASPix_Config();
	matrix->MatrixDACConfig = new ATLASPix_Config();
	matrix->VoltageDACConfig = new ATLASPix_Config();


	//DAC Block 1 for DIgital Part
	//AnalogDACs
	matrix->CurrentDACConfig->AddParameter("unlock",    4, ATLASPix_Config::LSBFirst, 0b1010); // unlock = x101
	matrix->CurrentDACConfig->AddParameter("BLResPix", "5,4,3,1,0,2",  5);
	matrix->CurrentDACConfig->AddParameter("ThResPix", "5,4,3,1,0,2",  0);
	matrix->CurrentDACConfig->AddParameter("VNPix", "5,4,3,1,0,2",  10);
	matrix->CurrentDACConfig->AddParameter("VNFBPix", "5,4,3,1,0,2", 20);
	matrix->CurrentDACConfig->AddParameter("VNFollPix", "5,4,3,1,0,2", 10);
	matrix->CurrentDACConfig->AddParameter("VNRegCasc", "5,4,3,1,0,2", 20);     //hier : VNHitbus
	matrix->CurrentDACConfig->AddParameter("VDel", "5,4,3,1,0,2", 10);
	matrix->CurrentDACConfig->AddParameter("VPComp", "5,4,3,1,0,2", 20);        //hier : VPHitbus
	matrix->CurrentDACConfig->AddParameter("VPDAC", "5,4,3,1,0,2",  0);
	matrix->CurrentDACConfig->AddParameter("VNPix2", "5,4,3,1,0,2",  0);
	matrix->CurrentDACConfig->AddParameter("BLResDig", "5,4,3,1,0,2",  5);
	matrix->CurrentDACConfig->AddParameter("VNBiasPix", "5,4,3,1,0,2",  0);
	matrix->CurrentDACConfig->AddParameter("VPLoadPix", "5,4,3,1,0,2",  5);
	matrix->CurrentDACConfig->AddParameter("VNOutPix", "5,4,3,1,0,2", 5);

	//DigitalDACs
	matrix->CurrentDACConfig->AddParameter("VPVCO", "5,4,3,1,0,2",  7);//5);//7);
	matrix->CurrentDACConfig->AddParameter("VNVCO", "5,4,3,1,0,2",  15);//15);
	matrix->CurrentDACConfig->AddParameter("VPDelDclMux", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VNDelDclMux", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VPDelDcl", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VNDelDcl", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VPDelPreEmp", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VNDelPreEmp", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VPDcl", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VNDcl", "5,4,3,1,0,2",  30);//30);
	matrix->CurrentDACConfig->AddParameter("VNLVDS", "5,4,3,1,0,2",  10);//10);
	matrix->CurrentDACConfig->AddParameter("VNLVDSDel", "5,4,3,1,0,2",  00);//10);
	matrix->CurrentDACConfig->AddParameter("VPPump", "5,4,3,1,0,2",  5);//5);

	matrix->CurrentDACConfig->AddParameter("nu", "1,0",  0);
	matrix->CurrentDACConfig->AddParameter("RO_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
	matrix->CurrentDACConfig->AddParameter("Ser_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
	matrix->CurrentDACConfig->AddParameter("Aur_res_n",     1, ATLASPix_Config::LSBFirst,  1);//1);  //for fastreadout start set 1
	matrix->CurrentDACConfig->AddParameter("sendcnt",     1, ATLASPix_Config::LSBFirst,  0);//0);
	matrix->CurrentDACConfig->AddParameter("resetckdivend", "3,2,1,0",  0);//2);
	matrix->CurrentDACConfig->AddParameter("maxcycend", "5,4,3,2,1,0",  63);//10); // probably 0 not allowed
	matrix->CurrentDACConfig->AddParameter("slowdownend", "3,2,1,0",  0);//1);
	matrix->CurrentDACConfig->AddParameter("timerend", "3,2,1,0",  1);//8); // darf nicht 0!! sonst werden debug ausgaben verschluckt
	matrix->CurrentDACConfig->AddParameter("ckdivend2", "5,4,3,2,1,0",  0);//1);
	matrix->CurrentDACConfig->AddParameter("ckdivend", "5,4,3,2,1,0",  0);//1);
	matrix->CurrentDACConfig->AddParameter("VPRegCasc", "5,4,3,1,0,2",  20);
	matrix->CurrentDACConfig->AddParameter("VPRamp", "5,4,3,1,0,2",  0); // was 4, off for HB/Thlow usage and fastreadout
	matrix->CurrentDACConfig->AddParameter("VNcompPix", "5,4,3,1,0,2",  10);     //VNComparator
	matrix->CurrentDACConfig->AddParameter("VPFoll", "5,4,3,1,0,2",  10);
	matrix->CurrentDACConfig->AddParameter("VNDACPix", "5,4,3,1,0,2",  8);
	matrix->CurrentDACConfig->AddParameter("VPBiasRec", "5,4,3,1,0,2",  30);
	matrix->CurrentDACConfig->AddParameter("VNBiasRec", "5,4,3,1,0,2",  30);
	matrix->CurrentDACConfig->AddParameter("Invert",     1, ATLASPix_Config::LSBFirst, 0);// 0);
	matrix->CurrentDACConfig->AddParameter("SelEx",     1, ATLASPix_Config::LSBFirst,  1);//1); //activated external clock input
	matrix->CurrentDACConfig->AddParameter("SelSlow",     1, ATLASPix_Config::LSBFirst,  1);//1);
	matrix->CurrentDACConfig->AddParameter("EnPLL",     1, ATLASPix_Config::LSBFirst,  0);//0);
	matrix->CurrentDACConfig->AddParameter("TriggerDelay",     10, ATLASPix_Config::LSBFirst,  0);
	matrix->CurrentDACConfig->AddParameter("Reset", 1, ATLASPix_Config::LSBFirst, 0);
	matrix->CurrentDACConfig->AddParameter("ConnRes",     1, ATLASPix_Config::LSBFirst,  1);//1);   //activates termination for output lvds
	matrix->CurrentDACConfig->AddParameter("SelTest",     1, ATLASPix_Config::LSBFirst,  0);
	matrix->CurrentDACConfig->AddParameter("SelTestOut",     1, ATLASPix_Config::LSBFirst,  0);

	if(matrix->type==1){

	//Column Register
		for (int col = 0; col < matrix->ncol; col++)
		{
			std::string s = to_string(col);
			matrix->MatrixDACConfig->AddParameter("RamDown"+s, 4, ATLASPix_Config::LSBFirst,  0b000); //0b1011
			matrix->MatrixDACConfig->AddParameter("colinjDown"+s, 1, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("hitbusDown"+s, 1, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("unusedDown"+s, 2, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("RamUp"+s, 4, ATLASPix_Config::LSBFirst,  0b000); //0b1011
			matrix->MatrixDACConfig->AddParameter("colinjUp"+s, 1, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("hitbusUp"+s, 1, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("unusedUp"+s, 2, ATLASPix_Config::LSBFirst,  0);
		}
	}

	else
		{
		for (int col = 0; col < matrix->ndoublecol; col++)
		{
			std::string s = to_string(col);
			matrix->MatrixDACConfig->AddParameter("RamL"+s, 3, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("colinjL"+s, 1, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("RamR"+s, 3, ATLASPix_Config::LSBFirst,  0);
			matrix->MatrixDACConfig->AddParameter("colinjR"+s, 1, ATLASPix_Config::LSBFirst,  0);
		}

	}


	//Row Register
	for (int row = 0; row < matrix->nrow; row++)
	{
		std::string s = to_string(row);
		matrix->MatrixDACConfig->AddParameter("writedac"+s, 1, ATLASPix_Config::LSBFirst, 0);
		matrix->MatrixDACConfig->AddParameter("unused"+s,   3, ATLASPix_Config::LSBFirst, 0);
		matrix->MatrixDACConfig->AddParameter("rowinjection"+s, 1, ATLASPix_Config::LSBFirst, 0);

		if(row==0){
		matrix->MatrixDACConfig->AddParameter("analogbuffer"+s, 1, ATLASPix_Config::LSBFirst, 0);
		}
		else{
		matrix->MatrixDACConfig->AddParameter("analogbuffer"+s, 1, ATLASPix_Config::LSBFirst, 0);

		}
	}



	matrix->VoltageDACConfig->AddParameter("BLPix", 8,ATLASPix_Config::LSBFirst, floor(255 * matrix->BLPix/1.8));
	matrix->VoltageDACConfig->AddParameter("nu2", 2, ATLASPix_Config::LSBFirst, matrix->nu2);
	matrix->VoltageDACConfig->AddParameter("ThPix", 8, ATLASPix_Config::LSBFirst, floor(255 * matrix->ThPix/1.8));
	matrix->VoltageDACConfig->AddParameter("nu3", 2, ATLASPix_Config::LSBFirst, matrix->nu3);

}


void ATLASPix::Fill_SR(ATLASPixMatrix *matrix)
{
    uint32_t buffer =0;
    uint32_t cnt =0;
    uint32_t nbits =0;
    matrix->VoltageDACBits = matrix->VoltageDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
    matrix->CurrentDACbits = matrix->CurrentDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
    matrix->MatrixBits = matrix->MatrixDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);
    //CurrentDACbits = CurrentDACConfig->GenerateBitVector(ATLASPix_Config::GlobalInvertedMSBFirst);


    std::vector<bool> allbits;
    allbits.insert( allbits.end(), matrix->VoltageDACBits.begin(), matrix->VoltageDACBits.end() );
    allbits.insert( allbits.end(), matrix->CurrentDACbits.begin(), matrix->CurrentDACbits.end() );
    allbits.insert( allbits.end(),  matrix->MatrixBits.begin(),  matrix->MatrixBits.end() );
    allbits.insert( allbits.end(), matrix->CurrentDACbits.begin(), matrix->CurrentDACbits.end() );

//    std::cout << matrix->VoltageDACBits.size() << std::endl;;
//    std::cout << matrix->CurrentDACbits.size() << std::endl;;
//    std::cout << matrix->MatrixBits.size() << std::endl;;
//    std::cout << allbits.size() << std::endl;;


    matrix->Registers.clear();

    for (auto i = allbits.begin(); i != allbits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 matrix->Registers.push_back(buffer);
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;
       nbits++;
     }


	 matrix->Registers.push_back(buffer);


     //std::cout << "size of shift buffer " << matrix->Registers.size() << std::endl;
     //std::cout << "number of bits " << nbits << std::endl;

}


void ATLASPix::Shift_SR(ATLASPixMatrix *matrix){


	void* control_base = _hal->getMappedMemoryRW(ATLASPix_CONTROL_BASE_ADDRESS, ATLASPix_CONTROL_MAP_SIZE, ATLASPix_RAM_address_MASK);


	volatile uint32_t* RAM_address = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x0);
	volatile uint32_t* RAM_content = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x4);
	volatile uint32_t* RAM_write_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x8);
	volatile uint32_t* RAM_reg_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0xC);
	volatile uint32_t* RAM_shift_limit = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x10);
	volatile uint32_t* Config_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x14);
	//volatile uint32_t* global_reset = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x18);
	volatile uint32_t* output_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + 0x1C);

	*Config_flag = 0;

	*RAM_reg_limit = matrix->nSRbuffer;
	*RAM_shift_limit = matrix->extraBits;

	uint32_t tmp=0;

	for(uint32_t i =0;i<=matrix->nSRbuffer;i++){
		*RAM_address =i;
		tmp=matrix->Registers[i];
		*RAM_content = tmp;
		usleep(10);
		*RAM_write_enable =0x1;
		usleep(10);
		*RAM_write_enable =0x0;

		 //std::cout << matrix->Registers[i] << " " ;
		 //std::cout << std::hex << matrix->Registers[i] << " ";
		 //this->printBits(sizeof(matrix->Registers[i]),&matrix->Registers[i]);

	};

	*output_enable = matrix->SRmask;
	usleep(10);


	*Config_flag = 0x1;
	usleep(30000);
	*Config_flag = 0;
	*output_enable = 0x0;
	//powerStatusLog();

}


void ATLASPix::ProgramSR(ATLASPixMatrix *matrix){

	this->Fill_SR(matrix);
 	this->Shift_SR(matrix);

}



// Injection and pulser

void ATLASPix::resetPulser(){


	 void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
	 volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);
	 usleep(1);
	 *rst = 0x0;
	 usleep(1);
	 *rst = 0x1;
	 usleep(1);
	 *rst = 0x0;

}

void ATLASPix::setPulse(ATLASPixMatrix *matrix,uint32_t npulse,uint32_t n_up,uint32_t n_down, double voltage){

	LOG(logDEBUG) << " Set injection voltages ";
    _hal->setBiasRegulator(INJ_1,voltage);
    _hal->powerBiasRegulator(INJ_1, true);
    _hal->setBiasRegulator(INJ_2,voltage);
    _hal->powerBiasRegulator(INJ_2, true);
    _hal->setBiasRegulator(INJ_3,voltage);
    _hal->powerBiasRegulator(INJ_3, true);
    _hal->setBiasRegulator(INJ_4,voltage);
    _hal->powerBiasRegulator(INJ_4, true);

	 void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);

	 //volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);
	 volatile uint32_t* pulse_count = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x4);
	 volatile uint32_t* high_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x8);
	 volatile uint32_t* low_cnt = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0xC);
	 volatile uint32_t* output_enable = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x10);
	 //volatile uint32_t* rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x14);

	 *pulse_count = npulse;
	 *high_cnt = n_up;
	 *low_cnt = n_down;
	 *output_enable = 0xFFFFF;//matrix->PulserMask;

	 this->pulse_width = std::ceil(((npulse*n_up + npulse*n_down)*(1.0/160.0e6))/1e-6) + 10;
}

void ATLASPix::sendPulse(){

	 void* pulser_base = _hal->getMappedMemoryRW(ATLASPix_PULSER_BASE_ADDRESS, ATLASPix_PULSER_MAP_SIZE, ATLASPix_PULSER_MASK);
	 volatile uint32_t* inj_flag = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(pulser_base) + 0x0);

	 *inj_flag = 0x1;
	 usleep(pulse_width);
	 *inj_flag = 0x0;

}

void ATLASPix::resetCounters()
{


	 void* counter_base = _hal->getMappedMemoryRW(ATLASPix_COUNTER_BASE_ADDRESS, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK);

	 volatile uint32_t* cnt_rst = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x10);
	 volatile uint32_t* global_reset = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x14);


	 *global_reset = 0x0;
	 usleep(10);
	 *global_reset = 0x1;
	 usleep(10);
	 *global_reset = 0x0;



	 usleep(10);
	 *cnt_rst = 0x0;
	 usleep(1);
	 *cnt_rst = 0x1;
	 usleep(1);
	 *cnt_rst = 0x0;

}

int ATLASPix::readCounter(ATLASPixMatrix *matrix)
{
	void* counter_base = _hal->getMappedMemoryRW(ATLASPix_COUNTER_BASE_ADDRESS, ATLASPix_COUNTER_MAP_SIZE, ATLASPix_COUNTER_MASK);

	int value = 0;
	switch(matrix->counter){
					case 0 :
						 {volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x0);
						 value = *cnt_value;}
						 break;
					case 1:
						 {volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x4);
						 value = *cnt_value;}
						 break;
					case 2:
						 {volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0x8);
						 value = *cnt_value;
						 break;}
					case 3:
						 {volatile uint32_t* cnt_value = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(counter_base) + 0xC);
						 value = *cnt_value;
						 break;}
					default :
					{ std::cout << "NON-EXISTING COUNTER, RETURN -1"<< std::endl;
						 value = -1;}
	}

	return value;
}

void ATLASPix::SetPixelInjection(uint32_t col, uint32_t row,bool ana_state,bool hb_state){

	this->writePixelInj(this->simpleM1,col,row,ana_state,hb_state);
	this->writePixelInj(this->simpleM1ISO,col,row,ana_state,hb_state);
	this->writePixelInj(this->simpleM2,col,row,ana_state,hb_state);

}

void ATLASPix::SetPixelInjection(ATLASPixMatrix *matrix,uint32_t col, uint32_t row,bool ana_state,bool hb_state){

	this->writePixelInj(matrix,col,row,ana_state,hb_state);


}

void ATLASPix::writePixelInj(ATLASPixMatrix *matrix, uint32_t col, uint32_t row, bool ana_state,bool hb_state){


	std::string col_s;
	int double_col=0;

	bool inj = 0;

	if(ana_state==true or hb_state==true){
		inj=true;
	}



	if(matrix->type==1){
		std::string s = to_string(col);

		if(row<200){
		matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][row]); //0b1011
		matrix->MatrixDACConfig->SetParameter("colinjDown"+s,  inj);
		matrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  hb_state);
		matrix->MatrixDACConfig->SetParameter("unusedDown"+s,  3);
		matrix->MatrixDACConfig->SetParameter("colinjUp"+s,   inj);
		matrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
		matrix->MatrixDACConfig->SetParameter("unusedUp"+s,  3);

		}
		else{
		//std::cout << "up pixels" << std::endl;
		matrix->MatrixDACConfig->SetParameter("RamUp"+s,matrix->TDAC[col][row]); //0b1011
		matrix->MatrixDACConfig->SetParameter("colinjDown"+s,  inj);
		matrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
		matrix->MatrixDACConfig->SetParameter("unusedDown"+s,  3);
		matrix->MatrixDACConfig->SetParameter("colinjUp"+s,   inj);
		matrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  hb_state);
		matrix->MatrixDACConfig->SetParameter("unusedUp"+s,  3);


		}

	}
	else{

		double_col=int(std::floor(double(col)/2));
		col_s = to_string(double_col);
		if(col%2==0){
				matrix->MatrixDACConfig->SetParameter("RamL"+col_s,matrix->TDAC[col][row] & 0b111);
				matrix->MatrixDACConfig->SetParameter("colinjL"+col_s,inj);
		}
		else {
				matrix->MatrixDACConfig->SetParameter("RamR"+col_s, matrix->TDAC[col][row] & 0b111);
				matrix->MatrixDACConfig->SetParameter("colinjR"+col_s,inj);
		}


	}

	std::string row_s = to_string(row);
	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,inj);
	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,ana_state);
	this->ProgramSR(matrix);
	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
	this->ProgramSR(matrix);




}

void ATLASPix::pulse(uint32_t npulse,uint32_t tup,uint32_t tdown,double amplitude){

	  this->resetCounters();
	  this->setPulse(simpleM1,npulse,tup,tdown,amplitude);
	  //std::cout << "sending pulse" << std::endl;
	  this->sendPulse();
	  usleep(2000);

}


// TDAC Manipulation

void ATLASPix::initTDAC(ATLASPixMatrix *matrix,uint32_t value){

	uint32_t actual_value = value;
	if(value>7){
		std::cout << "Value out of range, setting TDAC to 7" << std::endl;
		actual_value = 7;
	}


	for(int col=0;col<matrix->ncol;col++){
		for(int row=0;row < matrix->nrow;row++){
			matrix->MASK[col][row]=0;
			matrix->TDAC[col][row] = (actual_value << 1) | matrix->MASK[col][row];
			}
		}
}

void ATLASPix::setOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	uint32_t actual_value = value;
	if(value>7){
		std::cout << "Value out of range, setting TDAC to 7" << std::endl;
		actual_value = 7;
	}
	matrix->MASK[col][row]=0;
	matrix->TDAC[col][row] = (actual_value << 1) | matrix->MASK[col][row];
}

void ATLASPix::setMaskPixel(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	matrix->MASK[col][row]=value;
	matrix->TDAC[col][row] = matrix->TDAC[col][row]  | matrix->MASK[col][row];
}

void ATLASPix::writeOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	this->setOneTDAC(matrix,col,row,value);

	if(matrix->type==1){

	//Column Register

			std::string s = to_string(col);
			matrix->MatrixDACConfig->SetParameter("colinjDown"+s,  0);
			matrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
			matrix->MatrixDACConfig->SetParameter("unusedDown"+s,  0);
			matrix->MatrixDACConfig->SetParameter("colinjUp"+s,   0);
			matrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
			matrix->MatrixDACConfig->SetParameter("unusedUp"+s,  0);

			if(row<200){
			matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][row]); //0b1011
			//matrix->MatrixDACConfig->SetParameter("RamUp"+s, 4, ATLASPix_Config::LSBFirst,  matrix->TDAC[col][row]); //0b1011
			}
			else{
			//matrix->MatrixDACConfig->SetParameter("RamDown"+s, 4, ATLASPix_Config::LSBFirst,  matrix->TDAC[col][row]); //0b1011
			matrix->MatrixDACConfig->SetParameter("RamUp"+s,matrix->TDAC[col][row]); //0b1011
			}

	}

	else
		{

    		int double_col=int(std::floor(double(col)/2));
    		std::string col_s = to_string(double_col);
    		if(col%2==0){
    				matrix->MatrixDACConfig->SetParameter("RamL"+col_s,matrix->TDAC[col][row]);
    				matrix->MatrixDACConfig->SetParameter("colinjL"+col_s,0);
    		}
    		else {
    				matrix->MatrixDACConfig->SetParameter("RamR"+col_s, matrix->TDAC[col][row]);
    				matrix->MatrixDACConfig->SetParameter("colinjR"+col_s,0);
    		}
		}

	std::string row_s = to_string(row);
	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

	this->ProgramSR(matrix);


	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

	this->ProgramSR(matrix);


}

void ATLASPix::writeUniformTDAC(ATLASPixMatrix *matrix,uint32_t value){


	std::string col_s;
	int double_col=0;


	this->initTDAC(matrix,value);

	//std::cout << "writing " <<  std::bitset<32>(value) << std::endl;

    	if(matrix->type==1){

    	//Column Register
    		for (int col = 0; col < matrix->ncol; col++)
    		{
    			std::string s = to_string(col);
    			matrix->MatrixDACConfig->SetParameter("colinjDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("unusedDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("colinjUp"+s,   0);
    			matrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("unusedUp"+s,  0);


    			matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][0] ); //0b1011
    			matrix->MatrixDACConfig->SetParameter("RamUp"+s, matrix->TDAC[col][0] ); //0b1011


    		}
    	}

    	else
    		{
    		for (int col = 0; col < matrix->ncol; col++)
    		{
        		double_col=int(std::floor(double(col)/2));
        		col_s = to_string(double_col);
        		if(col%2==0){
        				matrix->MatrixDACConfig->SetParameter("RamL"+col_s,matrix->TDAC[col][0] );
        				matrix->MatrixDACConfig->SetParameter("colinjL"+col_s,0);
        		}
        		else {
        				matrix->MatrixDACConfig->SetParameter("RamR"+col_s, matrix->TDAC[col][0] );
        				matrix->MatrixDACConfig->SetParameter("colinjR"+col_s,0);
        		}


    		}
    		};

    	for (int row = 0; row < matrix->nrow; row++){

    	//std::cout << "processing row : " << row << std::endl;
    	std::string row_s = to_string(row);
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
    	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);
    	};


    	this->ProgramSR(matrix);


    	for (int row = 0; row < matrix->nrow; row++){

    	//std::cout << "processing row : " << row << std::endl;
    	std::string row_s = to_string(row);
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
    	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
    	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);
    	};


    	this->ProgramSR(matrix);

    	for (int row = 0; row < matrix->nrow; row++){

    	//std::cout << "processing row : " << row << std::endl;
    	std::string row_s = to_string(row);
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
    	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

    	};

    	this->ProgramSR(matrix);


}

void ATLASPix::setAllTDAC(uint32_t value){



	//std::cout << "before setall TDAC = " <<  simpleM1->TDAC[0][0]<< std::endl;

	this->writeUniformTDAC(simpleM1,value);
	//this->writeUniformTDAC(simpleM1ISO,value);
	//this->writeUniformTDAC(simpleM2,value);


	//std::cout << "after setall TDAC = " << simpleM1->TDAC[0][0]<< std::endl;


}

void ATLASPix::loadAllTDAC(std::string filename){


		std::ifstream myfile(filename);
		//myfile.open(filename);
		char data[100];

		uint32_t col,row,TDAC,mask;

		while(!myfile.eof()){


			myfile >> col >> row >> TDAC >> mask ;

			//std::cout  << " ---------------------------- " << std::endl;
			//std::cout  << " col: " << col << " row: " << row << " TDAC: " << TDAC << " mask : " << mask << std::endl;


			this->setOneTDAC(simpleM1,col,row,TDAC);
			this->setMaskPixel(simpleM1,col,row,mask);

			//std::cout  << " col: " << col << " row: " << row << " TDAC: " << this->simpleM1->TDAC[col][row] << " mask : " <<  this->simpleM1->MASK[col][row] << std::endl;
			//std::cout  << " ---------------------------- " << std::endl;

		}
		this->writeAllTDAC(simpleM1);

}


void ATLASPix::LoadTDAC(std::string filename){

this->loadAllTDAC(filename);
}


void ATLASPix::writeAllTDAC(ATLASPixMatrix *matrix){


	std::string col_s;
	int double_col=0;

	//std::cout << "i am here" << std::endl;

	for (int row = 0; row < matrix->nrow; row++){
    	if(matrix->type==1){

    	//Column Register
    		for (int col = 0; col < matrix->ncol; col++)
    		{
    			std::string s = to_string(col);
    			matrix->MatrixDACConfig->SetParameter("colinjDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("unusedDown"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("colinjUp"+s,   0);
    			matrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
    			matrix->MatrixDACConfig->SetParameter("unusedUp"+s,  0);

    			if(row<200){
    			matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][row]); //0b1011
    			matrix->MatrixDACConfig->SetParameter("RamUp"+s,matrix->TDAC[col][row]); //0b1011


    			}
    			else{
    			matrix->MatrixDACConfig->SetParameter("RamUp"+s,matrix->TDAC[col][row]); //0b1011
    			matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][row]); //0b1011

    			}

    		}
    	}

    	else
    		{
    		for (int col = 0; col < matrix->ncol; col++)
    		{
        		double_col=int(std::floor(double(col)/2));
        		col_s = to_string(double_col);
        		if(col%2==0){
        				matrix->MatrixDACConfig->SetParameter("RamL"+col_s,matrix->TDAC[col][row]);
        				matrix->MatrixDACConfig->SetParameter("colinjL"+col_s,0);
        		}
        		else {
        				matrix->MatrixDACConfig->SetParameter("RamR"+col_s, matrix->TDAC[col][row]);
        				matrix->MatrixDACConfig->SetParameter("colinjR"+col_s,0);
        		}


    		}
    		};

    	if(row%25==0){std::cout << "processing row : " << row << std::endl;}
    	std::string row_s = to_string(row);

    	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
    	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

    	//Toggle the line
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
    	this->ProgramSR(matrix);

    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
    	this->ProgramSR(matrix);

    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
    	this->ProgramSR(matrix);


    };

}

// Tuning

void ATLASPix::tune(ATLASPixMatrix *matrix, double vmax,int nstep, int npulses, bool tuning_verification) {
	LOG(logINFO) << "Tunning " << DEVICE_NAME;

	for (int TDAC_value = 0; TDAC_value < 8; TDAC_value++){
		initTDAC(matrix, TDAC_value);
		writeAllTDAC(matrix);
		ComputeSCurves(matrix,vmax, nstep, npulses, 100, 100);
		//s_curve plots
	}

	//double threshold_target = 0;
	const int cols = matrix->ncol;
	const int rows = matrix->nrow;
	int TDAC_map[cols][rows] = {0,0};
	//threshold_target calculation;
	//pixel TDAC interpolation for target
	//==> new, tuned, TDAC map

	for(int col=0;col<matrix->ncol;col++){
			for(int row=0;row<matrix->nrow;row++){
				matrix->TDAC[col][row] = TDAC_map[col][row];
			}
	}
	writeAllTDAC(matrix);
	if(tuning_verification == true){
		ComputeSCurves(matrix,0.5,50,128,100,100);
		//S_curve plots + threshold distribution
	}

}

void ATLASPix::ComputeSCurves(ATLASPixMatrix *matrix,double vmax,int nstep, int npulses,int tup,int tdown){

    std::clock_t start;
    double duration;

    start = std::clock();
    const int steps = nstep;
    const int cols = matrix->ncol;
    const int rows = matrix->nrow;
	double s_curves[steps][cols][rows] = {0, 0, 0};

	int step = 0;
    for(double v=0;v<=vmax;v+=(vmax/nstep)){
		setPulse(matrix,npulses,tup,tdown,v);
		std::cout << "  bias :" << v << "V"<< std::endl;

		for(int col=0;col< matrix->ncol; col++){
			for(int row=0;row< matrix->nrow; row++){
				sendPulse();
				int sent = this->readCounter(0);
				int rec = this->readCounter(matrix);
				double ratio = double(rec)/sent;
				resetCounters();
				s_curves[step][col][row] = ratio;
			}
		}
		step++;
	}
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	std::cout << "duration : " << duration << "s" << std::endl;

}

void ATLASPix::doSCurve(uint32_t col,uint32_t row,double vmin,double vmax,uint32_t npulses,uint32_t npoints){


	//this->SetPixelInjection(simpleM1,0,0,1,1);
	//this->SetPixelInjection(simpleM1,0,0,0,0);

	this->SetPixelInjection(simpleM1,col,row,1,1);
	this->resetCounters();

	int cnt=0;

	double vinj=vmin;
	double dv = (vmax-vmin)/(npoints-1);

	for(int i=0;i<npoints;i++){

		this->pulse(npulses,10000,10000,vinj);
		cnt=this->readCounter(simpleM1);
		std::cout << "V : " << vinj << " count : " << cnt << std::endl;
		vinj+=dv;
	}

	this->SetPixelInjection(col,row,0,0);

}

pearydata ATLASPix::getData(){



	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);

	 *fifo_config = 0b1;

	 for(int i=0;i<100;i++){

		 std::cout << "fifo_status" <<   std::bitset<32>(*fifo_status) << std::endl;
		 std::cout << "data word 1" <<   std::bitset<32>(*data) << std::endl;
		 std::cout << "data word 1" <<   std::bitset<32>(*data) << std::endl;
		 std::cout << "leds " <<   std::bitset<32>(*leds) << std::endl;


	 }

	 pearydata dummy;
	 return dummy;

}

void ATLASPix::doSCurves(double vmin,double vmax,uint32_t npulses,uint32_t npoints){

	std::cout << "Ok lets get started" << std::endl;
	int cnt=0;
	double vinj=vmin;
	double dv = (vmax-vmin)/(npoints-1);
	std::cout << "vmin : " << vmin << " vmax : " << vmax << " dV  : "<< dv << std::endl;

	//Setting Time Stamp in the file name
	std::time_t t = std::time(NULL);
	std::tm *ptm = std::localtime(&t);
	std::stringstream ss;
	ss << "PEARYDATA/ATLASPixGradeA_02/" <<"_"<< ptm->tm_year+1900 <<"_"<< ptm->tm_mon+1 <<"_"<< ptm->tm_mday <<"@"<< ptm->tm_hour+1 <<"_"<< ptm->tm_min+1 << "_"<<ptm->tm_sec+1;

	std::string cmd;
	cmd+="mkdir -p ";
	cmd+=ss.str();
	const int dir_err = system(cmd.c_str());

	std::string filename;
	filename+=ss.str();
	filename+="/";
	filename+="M1_VNDAC_";
	filename+=std::to_string(simpleM1->CurrentDACConfig->GetParameter("VNDACPix"));
	filename+="_TDAC_";
	filename+=std::to_string(simpleM1->TDAC[0][0]>>1);
	//filename+=ss.str();
	filename+=".txt";


	std::cout << "writing to file : " << filename << std::endl;


	std::ofstream myfile;
	myfile.open (filename);

	myfile << npoints << std::endl;

    std::clock_t start;
    double duration;

	for(int col=0;col< simpleM1->ncol; col++){
		for(int row=0;row< simpleM1->nrow; row++){



		    //start = std::clock();

			if(row%5==0){std::cout << "X: " << col << " Y: " << row << "\n" ;}


			vinj=vmin;
			//this->SetPixelInjection(simpleM1,0,0,1,1);
			//this->SetPixelInjection(simpleM1,0,0,0,0);

			this->SetPixelInjection(simpleM1,col,row,1,1);
			this->resetCounters();

			for(int i=0;i<npoints;i++){
				this->pulse(npulses,1000,1000,vinj);
				//cnt=this->readCounter(simpleM1ISO);
				cnt=this->readCounter(simpleM1);
				//this->getData();
				myfile << vinj << " " << cnt << " ";
				//std::cout << "V : " << vinj << " count : " << cnt << std::endl;
				vinj+=dv;
			}

			this->SetPixelInjection(simpleM1,col,row,0,0);
			myfile << std::endl;

			//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
			//std::cout << duration << " s \n" ;


		}}

	myfile.close();

}


void ATLASPix::doSCurves(std::string basefolder,double vmin,double vmax,uint32_t npulses,uint32_t npoints){

	std::cout << "Ok lets get started" << std::endl;
	int cnt=0;
	double vinj=vmin;
	double dv = (vmax-vmin)/(npoints-1);
	std::cout << "vmin : " << vmin << " vmax : " << vmax << " dV  : "<< dv << std::endl;

	std::string filename;
	filename+=basefolder;
	filename+="/";
	filename+="M1_VNDAC_";
	filename+=std::to_string(simpleM1->CurrentDACConfig->GetParameter("VNDACPix"));
	filename+="_TDAC_";
	filename+=std::to_string(simpleM1->TDAC[0][0]>>1);
	//filename+=ss.str();
	filename+=".txt";


	std::cout << "writing to file : " << filename << std::endl;


	std::ofstream myfile;
	myfile.open (filename);

	myfile << npoints << std::endl;

    std::clock_t start;
    double duration;

	for(int col=0;col< simpleM1->ncol; col++){
		for(int row=0;row< simpleM1->nrow; row++){



		    //start = std::clock();

			if(row%5==0){std::cout << "X: " << col << " Y: " << row << "\n" ;}


			vinj=vmin;
			//this->SetPixelInjection(simpleM1,0,0,1,1);
			//this->SetPixelInjection(simpleM1,0,0,0,0);

			this->SetPixelInjection(simpleM1,col,row,1,1);
			this->resetCounters();

			for(int i=0;i<npoints;i++){
				this->pulse(npulses,1000,1000,vinj);
				//cnt=this->readCounter(simpleM1ISO);
				cnt=this->readCounter(simpleM1);
				//this->getData();
				myfile << vinj << " " << cnt << " ";
				//std::cout << "V : " << vinj << " count : " << cnt << std::endl;
				vinj+=dv;
			}

			this->SetPixelInjection(simpleM1,col,row,0,0);
			myfile << std::endl;

			//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
			//std::cout << duration << " s \n" ;


		}}

	myfile.close();

}



void ATLASPix::TDACScan(std::string basefolder,int VNDAC,int step,double vmin,double vmax,uint32_t npulses,uint32_t npoints){

	int mat=1;

	this->WriteConfig(basefolder+"/config");

	ATLASPixMatrix *matrix;
	if (mat==0){
		matrix=simpleM2;
	}
	else if (mat==1) {
		matrix=simpleM1;
	}
	else{
		matrix=simpleM1ISO;

	}

	matrix->CurrentDACConfig->SetParameter("VNDACPix",VNDAC);


	for(int tdac=0;tdac<=8;tdac+=step){


		this->setAllTDAC(tdac);
		this->doSCurves(basefolder, vmin, vmax, npulses,npoints);

	}




}



void ATLASPix::doNoiseCurve(uint32_t col,uint32_t row){


	this->SetPixelInjection(col,row,1,1);
	this->resetCounters();

	int cnt=0;

	double threshold = 0.88;
	this->setThreshold(threshold);

	while(cnt<1000){
		this->resetCounters();
		usleep(1000);
		this->setThreshold(threshold);
		std::cout << "M1  : " << this->readCounter(simpleM1)<< std::endl;
		std::cout << "M1ISO  : " << this->readCounter(simpleM1ISO)<< std::endl;
		std::cout << "M2  : " << this->readCounter(simpleM2)<< std::endl;

		cnt = this->readCounter(simpleM1ISO);
		threshold -=0.001;


	}

	std::cout << "noise floor at : " << threshold+0.001 << std::endl;

}


//CaR Board related


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
  std::cout << '\n';

 // Power rails: Before Biasing
 // LOG(logINFO) << DEVICE_NAME << ": Powering details before biasing";
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(PWR_OUT_4, _config.Get("vddd", ATLASPix_VDDD), _config.Get("vddd_current", ATLASPix_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_4, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(PWR_OUT_3, _config.Get("vdda", ATLASPix_VDDA), _config.Get("vdda_current", ATLASPix_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_3, true);

  LOG(logDEBUG) << " VSSA";
  _hal->setVoltageRegulator(PWR_OUT_2, _config.Get("vssa", ATLASPix_VSSA), _config.Get("vssa_current", ATLASPix_VSSA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_2, true);

  //this->powerStatusLog();
  // Bias voltages m2:
  LOG(logDEBUG) << " GNDDacPix m2 ";
  _hal->setBiasRegulator(BIAS_6, _config.Get("GndDACPix_M2", ATLASPix_GndDACPix_M2));
  _hal->powerBiasRegulator(BIAS_6, true);

  LOG(logDEBUG) << " VMinusPix m2 ";
  _hal->setBiasRegulator(BIAS_4, _config.Get("VMinusPix_M2", ATLASPix_VMinusPix_M2));
  _hal->powerBiasRegulator(BIAS_4, true);

  LOG(logDEBUG) << " GatePix m2 ";
  _hal->setBiasRegulator(BIAS_1, _config.Get("GatePix_M2", ATLASPix_GatePix_M2));
  _hal->powerBiasRegulator(BIAS_1, true);

  simpleM2->GNDDACPix=_config.Get("GndDACPix_M2", ATLASPix_GndDACPix_M2);
  simpleM2->VMINUSPix=_config.Get("GndDACPix_M2", ATLASPix_VMinusPix_M2);
  simpleM2->GatePix=_config.Get("GndDACPix_M2", ATLASPix_GatePix_M2);



  // Bias voltages m1:
  LOG(logDEBUG) << " GNDDacPix m1 ";
  _hal->setBiasRegulator(BIAS_9, _config.Get("GndDACPix_M1", ATLASPix_GndDACPix_M1));
  _hal->powerBiasRegulator(BIAS_9, true);

  LOG(logDEBUG) << " VMinusPix m1";
  _hal->setBiasRegulator(BIAS_5, _config.Get("VMinusPix_M1", ATLASPix_VMinusPix_M1));
  _hal->powerBiasRegulator(BIAS_5, true);

  LOG(logDEBUG) << " GatePix m1 ";
  _hal->setBiasRegulator(BIAS_2, _config.Get("GatePix_M1", ATLASPix_GatePix_M1));
  _hal->powerBiasRegulator(BIAS_2, true);

  simpleM1->GNDDACPix=_config.Get("GndDACPix_M1", ATLASPix_GndDACPix_M1);
  simpleM1->VMINUSPix=_config.Get("GndDACPix_M1", ATLASPix_VMinusPix_M1);
  simpleM1->GatePix=_config.Get("GndDACPix_M1", ATLASPix_GatePix_M1);


  // Bias voltages m1:
  LOG(logDEBUG) << " GNDDacPix m1 iso";
  _hal->setBiasRegulator(BIAS_12, _config.Get("GndDACPix_M1ISO", ATLASPix_GndDACPix_M1ISO));
  _hal->powerBiasRegulator(BIAS_12, true);

  LOG(logDEBUG) << " VMinusPix m1 iso ";
  _hal->setBiasRegulator(BIAS_8, _config.Get("VMinusPix_M1ISO", ATLASPix_VMinusPix_M1ISO));
  _hal->powerBiasRegulator(BIAS_8, true);

  LOG(logDEBUG) << " GatePix m1 iso ";
  _hal->setBiasRegulator(BIAS_3, _config.Get("GatePix_M1ISO", ATLASPix_GatePix_M1ISO));
  _hal->powerBiasRegulator(BIAS_3, true);


  simpleM1ISO->GNDDACPix=_config.Get("GndDACPix_M1ISO", ATLASPix_GndDACPix_M1ISO);
  simpleM1ISO->VMINUSPix=_config.Get("GndDACPix_M1ISO", ATLASPix_VMinusPix_M1ISO);
  simpleM1ISO->GatePix=_config.Get("GndDACPix_M1ISO", ATLASPix_GatePix_M1ISO);




  // BL and Threshold from ext

  LOG(logDEBUG) << " BLPix m1 ISO ";
  _hal->setBiasRegulator(BIAS_20, _config.Get("BLPix_M1ISO", ATLASPix_BLPix_M1ISO));
  _hal->powerBiasRegulator(BIAS_20, true);

  LOG(logDEBUG) << " BLPix m1  ";
  _hal->setBiasRegulator(BIAS_17, _config.Get("BLPix_M1", ATLASPix_BLPix_M1));
  _hal->powerBiasRegulator(BIAS_17, true);

  LOG(logDEBUG) << " BLPix m2  ";
  _hal->setBiasRegulator(BIAS_23, _config.Get("BLPix_M2", ATLASPix_BLPix_M2));
  _hal->powerBiasRegulator(BIAS_23, true);




  LOG(logDEBUG) << " ThPix m1 ISO ";
  _hal->setBiasRegulator(BIAS_28, _config.Get("ThPix_M1ISO", ATLASPix_ThPix_M1ISO));
  _hal->powerBiasRegulator(BIAS_28, true);


  LOG(logDEBUG) << " ThPix m1  ";
  _hal->setBiasRegulator(BIAS_25, _config.Get("ThPix_M1", ATLASPix_ThPix_M1));
  _hal->powerBiasRegulator(BIAS_25, true);


  LOG(logDEBUG) << " ThPix m2 ";
  _hal->setBiasRegulator(BIAS_31, _config.Get("ThPix_M2", ATLASPix_ThPix_M2));
  _hal->powerBiasRegulator(BIAS_31, true);



  simpleM1->BLPix= _config.Get("BLPix_M1", ATLASPix_BLPix_M1);
  simpleM2->BLPix= _config.Get("BLPix_M2", ATLASPix_BLPix_M2);
  simpleM1ISO->BLPix= _config.Get("BLPix_M1ISO", ATLASPix_BLPix_M1ISO);

  simpleM1->ThPix= _config.Get("ThPix_M1", ATLASPix_ThPix_M1);
  simpleM2->ThPix= _config.Get("ThPix_M2", ATLASPix_ThPix_M2);
  simpleM1ISO->ThPix= _config.Get("ThPix_M1ISO", ATLASPix_ThPix_M1ISO);



  std::cout << '\n';

  /*// Power rails: After Biasing
  LOG(logINFO) << DEVICE_NAME << ": Powering details after biasing";
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator(PWR_OUT_4, _config.Get("vddd", ATLASPix_VDDD), _config.Get("vddd_current", ATLASPix_VDDD_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_4, true);

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator(PWR_OUT_3, _config.Get("vdda", ATLASPix_VDDA), _config.Get("vdda_current", ATLASPix_VDDA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_3, true);

  LOG(logDEBUG) << " VSSA";
  _hal->setVoltageRegulator(PWR_OUT_2, _config.Get("vssa", ATLASPix_VSSA), _config.Get("vssa_current", ATLASPix_VSSA_CURRENT));
  _hal->powerVoltageRegulator(PWR_OUT_2, true);

  //usleep(100000);
  //powerStatusLog()*/


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

  LOG(logDEBUG) << "Turning off GNDDacPix_M1";
  _hal->powerBiasRegulator(BIAS_6, false);

  LOG(logDEBUG) << "Turning off VMinusPix_M1";
  _hal->powerBiasRegulator(BIAS_4, false);

  LOG(logDEBUG) << "Turning off GatePix_M1";
  _hal->powerBiasRegulator(BIAS_1, false);

  LOG(logDEBUG) << "Turning off GNDDacPix_M2";
  _hal->powerBiasRegulator(BIAS_9, false);

  LOG(logDEBUG) << "Turning off VMinusPix_M2";
  _hal->powerBiasRegulator(BIAS_5, false);

  LOG(logDEBUG) << "Turning off GatePix_M2";
  _hal->powerBiasRegulator(BIAS_2, false);

  LOG(logDEBUG) << "Turning off GNDDacPix_M1ISO";
  _hal->powerBiasRegulator(BIAS_12, false);

  LOG(logDEBUG) << "Turning off VMinusPix_M1ISO";
  _hal->powerBiasRegulator(BIAS_8, false);

  LOG(logDEBUG) << "Turning off GatePix_M1ISO";
  _hal->powerBiasRegulator(BIAS_3, false);

  //this->powerStatusLog();

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

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";

  LOG(logINFO) << "VSSA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_2) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_2) << "A";

}



void ATLASPix::WriteConfig(std::string basename){

	int mat=1;

	ATLASPixMatrix *matrix;
	if (mat==0){
		matrix=simpleM2;
	}
	else if (mat==1) {
		matrix=simpleM1;
	}
	else{
		matrix=simpleM1ISO;

	}


	std::ofstream myfile;
	myfile.open(basename + ".cfg" );


	static std::vector<std::string> VoltageDACs;
	VoltageDACs = {"BLPix", "nu2","ThPix","nu3"};

	for(auto const& value: VoltageDACs) {
		myfile << std::left << std::setw(20) << value << " " << matrix->VoltageDACConfig->GetParameter(value) << std::endl;
	}

	static std::vector<std::string> CurrentDACs;
	CurrentDACs = {"unlock","BLResPix","ThResPix","VNPix","VNFBPix","VNFollPix","VNRegCasc","VDel","VPComp","VPDAC","VNPix2","BLResDig","VNBiasPix","VPLoadPix","VNOutPix",
					"VPVCO","VNVCO","VPDelDclMux","VNDelDclMux","VPDelDcl","VNDelDcl","VPDelPreEmp","VNDelPreEmp","VPDcl","VNDcl","VNLVDS","VNLVDSDel","VPPump","nu",
					"RO_res_n","Ser_res_n","Aur_res_n","sendcnt","resetckdivend","maxcycend","slowdownend","timerend","ckdivend2","ckdivend","VPRegCasc","VPRamp","VNcompPix",
					"VPFoll","VPFoll","VNDACPix","VPBiasRec","VNBiasRec","Invert","SelEx","SelSlow","EnPLL","TriggerDelay","Reset","ConnRes","SelTest","SelTestOut"};


	for(auto const& value: CurrentDACs) {
		myfile << std::left << std::setw(20) << value << " " << matrix->CurrentDACConfig->GetParameter(value) << std::endl;
	}


	static std::vector<std::string> ExternalBias;
	ExternalBias = {"BLPix_ext", "ThPix_ext","VMINUSPix","GNDDACPix","GatePix"};

	myfile << std::left << std::setw(20)<< "ThPix_ext" << " " << matrix->ThPix << std::endl;
	myfile << std::left << std::setw(20)<< "BLPix_ext"<< " " << matrix->BLPix << std::endl;
	myfile << std::left << std::setw(20)<< "VMINUSPix" << " " << matrix->VMINUSPix << std::endl;
	myfile << std::left << std::setw(20)<< "GNDDACPix" << " " << matrix->GNDDACPix << std::endl;
	myfile << std::left << std::setw(20)<< "GatePix" << " " << matrix->GatePix << std::endl;



	std::ofstream TDACFile;
	TDACFile.open(basename + "_TDAC.cfg" );


	for(int col = 0;col<matrix->ncol;col++){
		for(int row = 0;row<matrix->nrow;row++){

			TDACFile << std::left << std::setw(3) << col << " " << std::left << std::setw(3) << row << " "  << std::left << std::setw(2) << matrix->TDAC[col][row] << " " << std::left << std::setw(1) << matrix->MASK[col][row] << std::endl ;
		}
	}


	myfile.close();
	TDACFile.close();
}



void ATLASPix::LoadConfig(std::string basename){


	int mat=1;

	ATLASPixMatrix *matrix;
	if (mat==0){
		matrix=simpleM2;
	}
	else if (mat==1) {
		matrix=simpleM1;
	}
	else{
		matrix=simpleM1ISO;

	}

	std::ifstream configfile;
	configfile.open(basename + ".cfg" );

	static std::vector<std::string> ExternalBias;
	ExternalBias = {"BLPix_ext", "ThPix_ext","VMINUSPix","GNDDACPix","GatePix"};


	static std::vector<std::string> CurrentDACs;
	CurrentDACs = {"unlock","BLResPix","ThResPix","VNPix","VNFBPix","VNFollPix","VNRegCasc","VDel","VPComp","VPDAC","VNPix2","BLResDig","VNBiasPix","VPLoadPix","VNOutPix",
					"VPVCO","VNVCO","VPDelDclMux","VNDelDclMux","VPDelDcl","VNDelDcl","VPDelPreEmp","VNDelPreEmp","VPDcl","VNDcl","VNLVDS","VNLVDSDel","VPPump","nu",
					"RO_res_n","Ser_res_n","Aur_res_n","sendcnt","resetckdivend","maxcycend","slowdownend","timerend","ckdivend2","ckdivend","VPRegCasc","VPRamp","VNcompPix",
					"VPFoll","VPFoll","VNDACPix","VPBiasRec","VNBiasRec","Invert","SelEx","SelSlow","EnPLL","TriggerDelay","Reset","ConnRes","SelTest","SelTestOut"};


	static std::vector<std::string> VoltageDACs;
	VoltageDACs = {"BLPix", "nu2","ThPix","nu3"};

	std::string reg;
	int value=0;
	double bias=0;


	while(!configfile.eof()){

		configfile >> reg;

		std::cout << "processing : " << reg << std::endl;



		if (std::find(ExternalBias.begin(), ExternalBias.end(), reg) != ExternalBias.end()){
			configfile >> bias;

			if(reg=="BLPix_ext"){
				  _hal->setBiasRegulator(BIAS_17, bias);
				  _hal->powerBiasRegulator(BIAS_17, true);
			}
			else if(reg=="ThPix_ext"){
				  _hal->setBiasRegulator(BIAS_25,bias);
				  _hal->powerBiasRegulator(BIAS_25, true);
			}
			else if(reg=="VMINUSPix"){
				  _hal->setBiasRegulator(BIAS_5,bias);
				  _hal->powerBiasRegulator(BIAS_5, true);
			}
			else if(reg=="GNDDACPix"){
				  _hal->setBiasRegulator(BIAS_9,bias);
				  _hal->powerBiasRegulator(BIAS_9, true);
			}
			else if(reg=="GatePix"){
				  _hal->setBiasRegulator(BIAS_2,bias);
				  _hal->powerBiasRegulator(BIAS_2, true);
			}
			else {
				printf("unknown external bias register %s",reg);
			}
		}
		else if (std::find(CurrentDACs.begin(), CurrentDACs.end(), reg) != CurrentDACs.end()){
			configfile >> value;
			matrix->CurrentDACConfig->SetParameter(reg,value);
		}

		else if(std::find(VoltageDACs.begin(), VoltageDACs.end(), reg) != VoltageDACs.end()){
			configfile >> value;
			matrix->VoltageDACConfig->SetParameter(reg,value);
		}

		else {
			std::cout << "unknown register : " << reg << std::endl;
		}

	}

	this->ProgramSR(matrix);

	this->LoadTDAC(basename+"_TDAC.cfg");



}









caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  ATLASPix* mDevice = new ATLASPix(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}

//Data related


