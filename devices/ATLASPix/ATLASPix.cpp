/**
 * Caribou implementation for the ATLASPix
 */

#include "ATLASPix.hpp"

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>

#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

uint32_t reverseBits(uint8_t n) {
        uint32_t x;
        for(auto i = 7; n; ) {
            x |= (n & 1) << i;
            n >>= 1;
            -- i;
        }
        return x;
    }

uint32_t reverseBits64(uint64_t n) {
        uint32_t x;
        for(auto i = 63; n; ) {
            x |= (n & 1) << i;
            n >>= 1;
            -- i;
        }
        return x;
    }

// BASIC Configuration

struct pixelhit{

  uint32_t col=0;
  uint32_t row=0;
  uint32_t ts1=0;
  uint32_t ts2=0;
  uint64_t fpga_ts=0;
  uint32_t SyncedTS=0;
  uint32_t triggercnt;
  uint32_t ATPbinaryCnt;
  uint32_t ATPGreyCnt;

};

uint32_t grey_decode(uint32_t g)
{
    for (uint32_t bit = 1U <<31; bit > 1; bit >>= 1)
    {
        if (g & bit) g ^= bit >> 1;
    }
    return g;
}

pixelhit decodeHit(uint32_t hit,uint32_t TS, uint64_t fpga_ts,uint32_t SyncedTS,uint32_t triggercnt){

	 pixelhit tmp;
	 uint8_t buf=0;

	 tmp.fpga_ts=fpga_ts;
	 tmp.SyncedTS=SyncedTS;
	 tmp.triggercnt=triggercnt;
	 tmp.ATPbinaryCnt=(TS & 0xFFFF00) + grey_decode((TS & 0xFF));
	 tmp.ATPGreyCnt=TS & 0xFF;

	 buf=((hit >> 23) & 0xFF);

	 tmp.col=(24-((buf>>2)&0b00111111));
	 tmp.row= 255 -(reverseBits(hit & 0xFF));
	 tmp.ts2=hit>>18 & 0b00111111;
	 tmp.ts1=(hit>>8 & 0b1111111111)+  (hit>>16 & 0b11);
	 tmp.col=tmp.col & 0b11111;
	 tmp.row=tmp.row & 0b111111111;

	 if((buf>>1 & 0x1)==0)tmp.row+=200;
	 return tmp;
}

namespace Color {
    enum Code {
    	 FG_DEFAULT = 39, BOLD= 1, REVERSE=7,RESET=0, FG_BLACK = 30, FG_RED = 31, FG_GREEN = 32, FG_YELLOW = 33, FG_BLUE = 34, FG_MAGENTA = 35, FG_CYAN = 36, FG_LIGHT_GRAY = 37, FG_DARK_GRAY = 90, FG_LIGHT_RED = 91, FG_LIGHT_GREEN = 92, FG_LIGHT_YELLOW = 93, FG_LIGHT_BLUE = 94, FG_LIGHT_MAGENTA = 95, FG_LIGHT_CYAN = 96, FG_WHITE = 97, BG_RED = 41, BG_GREEN = 42, BG_BLUE = 44, BG_DEFAULT = 49
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
}

ATLASPix::ATLASPix(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), ATLASPix_DEFAULT_I2C) {


  //Configuring the clock to 160 MHz
  LOG(logINFO) << "Setting clock to 160MHz " << DEVICE_NAME;
  configureClock();

  _registers.add(ATLASPix_REGISTERS);

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


void ATLASPix::SetMatrix(std::string matrix){

	  // Set up periphery
	  _periphery.add("VDDD", PWR_OUT_4);
	  _periphery.add("VDDA", PWR_OUT_3);
	  _periphery.add("VSSA", PWR_OUT_2);



	this->theMatrix = new ATLASPixMatrix();

	char Choice;
	if(matrix=="M1"){ Choice = '1';}
	else if(matrix=="M2"){ Choice = '2';}
	else{ Choice = '3';};

	switch(Choice){
	case '1' :

		  this->theMatrix->BLPix=0.8;
		  this->theMatrix->ThPix=0.85;
		  this->theMatrix->ncol=ncol_m1;
		  this->theMatrix->ndoublecol=ncol_m1/2;
		  this->theMatrix->nrow=nrow_m1;
		  this->theMatrix->counter=2;
		  this->theMatrix->nSRbuffer = 104;
		  this->theMatrix->extraBits = 16;
		  this->theMatrix->SRmask=0x2;
		  this->theMatrix->PulserMask=0x2;
		  this->theMatrix->type=1;
		  this->theMatrix->regcase='1';

		  this->theMatrix->GNDDACPix=ATLASPix_GndDACPix_M1;
		  this->theMatrix->VMINUSPix=ATLASPix_VMinusPix_M1;
		  this->theMatrix->GatePix=ATLASPix_GatePix_M1;


		  _periphery.add("GNDDACPix", BIAS_9);
		  _periphery.add("VMinusPix", BIAS_5);
		  _periphery.add("GatePix", BIAS_2);
		  _periphery.add("ThPix",BIAS_25);
		  _periphery.add("BLPix",BIAS_17);
		  break;

	case '2' :

		  this->theMatrix->BLPix=0.8;
		  this->theMatrix->ThPix=0.85;
		  this->theMatrix->ncol=ncol_m2;
		  this->theMatrix->ndoublecol=ncol_m2/2;
		  this->theMatrix->nrow=nrow_m2;
		  this->theMatrix->counter=3;
		  this->theMatrix->nSRbuffer = 84;
		  this->theMatrix->extraBits = 0;
		  this->theMatrix->SRmask=0x1;
		  this->theMatrix->PulserMask=0x1;
		  this->theMatrix->type=2;
		  this->theMatrix->regcase='2';
		  this->theMatrix->GNDDACPix=ATLASPix_GndDACPix_M2;
		  this->theMatrix->VMINUSPix=ATLASPix_VMinusPix_M2;
		  this->theMatrix->GatePix=ATLASPix_GatePix_M2;


		  _periphery.add("GNDDACPix", BIAS_6);
		  _periphery.add("VMinusPix", BIAS_4);
		  _periphery.add("GatePix", BIAS_1);
		  _periphery.add("ThPix",BIAS_28);
		  _periphery.add("BLPix",BIAS_23);

		  break;
	case '3' :

		  this->theMatrix->BLPix=0.8;
		  this->theMatrix->ThPix=0.86+0.014;
		  this->theMatrix->ncol=ncol_m1iso;
		  this->theMatrix->ndoublecol=ncol_m1iso/2;
		  this->theMatrix->nrow=nrow_m1iso;
		  this->theMatrix->counter=1;
		  this->theMatrix->nSRbuffer = 104;
		  this->theMatrix->extraBits = 16;
		  this->theMatrix->SRmask=0x4;
		  this->theMatrix->PulserMask=0x4;
		  this->theMatrix->type=1;
		  this->theMatrix->regcase='3';
		  this->theMatrix->GNDDACPix=ATLASPix_GndDACPix_M1ISO;
		  this->theMatrix->VMINUSPix=ATLASPix_VMinusPix_M1ISO;
		  this->theMatrix->GatePix=ATLASPix_GatePix_M1ISO;

		  _periphery.add("GNDDACPix", BIAS_12);
		  _periphery.add("VMinusPix", BIAS_8);
		  _periphery.add("GatePix", BIAS_3);
		  _periphery.add("ThPix",BIAS_31);
		  _periphery.add("BLPix",BIAS_20);

		  break;
	default:
		  std::cout << "unknown matrix : " << matrix << std::endl;
		  break;


	}
	  this->Initialize_SR(this->theMatrix);


}




void ATLASPix::configure() {


 LOG(logINFO) << "Configuring " << DEVICE_NAME;

 this->resetPulser();
 this->resetCounters();

 //this->powerOn();
 usleep(1000);

 // Build the SR string with default values and shift in the values in the chip
 std::cout << "sending default config " << std::endl;
 this->ProgramSR(theMatrix);

  //this->ComputeSCurves(0,0.5,50,128,100,100);
 std::cout << "sending default TDACs " << std::endl;

 this->writeUniformTDAC(theMatrix,0b0000);
 this->setSpecialRegister("trigger_mode",2);
 this->setSpecialRegister("ro_enable",0);
 this->setSpecialRegister("armduration",2000);


 // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();


}



void ATLASPix::lock(){

	this->theMatrix->CurrentDACConfig->SetParameter("unlock",0x0);
	this->ProgramSR(theMatrix);

}

void ATLASPix::unlock(){


	this->theMatrix->CurrentDACConfig->SetParameter("unlock",0b1010);
	this->ProgramSR(theMatrix);


}

void ATLASPix::setThreshold(double threshold){


	theMatrix->VoltageDACConfig->SetParameter("ThPix",static_cast<int>(floor(255 * threshold/1.8)));

	this->ProgramSR(theMatrix);

	LOG(logDEBUG) << " ThPix ";
	this->setVoltage("ThPix",threshold);
	this->switchOn("ThPix");
	theMatrix->ThPix=threshold;


}

void ATLASPix::setSpecialRegister(std::string name, uint32_t value) {



		//UGLY HACK FIXME!!!
		//std::cout << '\n' << "***You have set " << name << " as " << std::dec << value <<  "***" << '\n' << '\n';
		char Choice;
		if(theMatrix->regcase=="M1"){ Choice = '1';}
		else if(theMatrix->regcase=="M2"){ Choice = '2';}
		else{ Choice = '3';};

		if(name == "unlock") {
	    // Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("unlock",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("unlock",0x0);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("unlock",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
	    }

		else if (name == "blrespix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "threspix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ThResPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}

		else if (name == "vnpix") {
		// Set DAC value here calling setParameter
			////std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			////std::cin >> Choice ;
			Choice = '1' ;
			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}

		else if (name == "vnfbpix") {
		// Set DAC value here calling setParameter
			////std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			////std::cin >> Choice ;
			Choice='1';
			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFBPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnfollpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNFollPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnregcasc") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vdel") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpcomp") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPComp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdac") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDAC",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnpix2") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNPix2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "blresdig") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("BLResDig",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnbiaspix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vploadpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPLoadPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnoutpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNOutPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpvco") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnvco") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNVCO",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdeldclmux") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndeldclmux") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDclMux",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdeldcl") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndeldcl") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdelpreemp") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndelpreemp") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDelPreEmp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpdcl") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndcl") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDcl",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnlvds") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDS",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnlvdsdel") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNLVDSDel",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vppump") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPPump",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "nu") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ro_res_n") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("RO_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ser_res_n") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Ser_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "aur_res_n") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Aur_res_n",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "sendcnt") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("sendcnt",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "resetckdivend") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("resetckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "maxcycend") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("maxcycend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "slowdownend") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("slowdownend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "timerend") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("timerend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ckdivend2") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "ckdivend") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ckdivend",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpregcasc") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRegCasc",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpramp") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPRamp",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vncomppix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNcompPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpfoll") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPFoll",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vndacpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			////std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNDACPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vpbiasrec") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VPBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "vnbiasrec") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("VNBiasRec",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "invert") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Invert",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "selex") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelEx",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "selslow") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelSlow",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "enpll") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("EnPLL",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "triggerdelay") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("TriggerDelay",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "reset") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("Reset",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			default : std::cout << "\nBad Input. Must be '1', '2' or '3'. Sorry, Try Again!" << '\n' << '\n';
			}
		}
		else if (name == "connres") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("ConnRes",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "seltest") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTest",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "seltestout") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("SelTestOut",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "blpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->VoltageDACConfig->SetParameter("BLPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "nu2") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu2",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "thpix") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->VoltageDACConfig->SetParameter("ThPix",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}
		else if (name == "nu3") {
		// Set DAC value here calling setParameter
			//std::cout << "\n\nWhich Matrix (1. Matrix M1-Simple or 3. Matrix M2-Triggered or 3. Matrix M1ISO)?  Enter 1-3: " ;
			//std::cin >> Choice ;

			switch(Choice)
			{
			case '1' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '2' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			case '3' :
				{
					theMatrix->CurrentDACConfig->SetParameter("nu3",value);
					this->ProgramSR(theMatrix);
					break ;
				}
			}
		}

		else if (name == "ro_enable") {

			 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

			 //volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
			 //volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
			 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
			 //volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
			 //volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

			 *fifo_config = (*fifo_config & 0xFFFFFFFE) + (value & 0b1);

		}
		else if (name == "trigger_mode") {

			 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

			 //volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
			 //volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
			 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
			 //volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
			 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

			 *fifo_config = (*fifo_config & 0x7) + (0b1000);
			 usleep(1);
			 *fifo_config = (*fifo_config & 0xFFFFFFF7) + (0b0000);

			 *ro = (*ro & 0xFFFCFFFF) + ((value << 16) & 0x30000);

		}
		else if (name == "armduration") {

			 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

			 //volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
			 //volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
			 //volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
			 //volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
			 volatile uint32_t* config2 = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x14);

			 *config2 = ((value) & 0xFFFFFF);

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
	matrix->CurrentDACConfig->AddParameter("resetckdivend", "3,2,1,0",  15);//2);
	matrix->CurrentDACConfig->AddParameter("maxcycend", "5,4,3,2,1,0",  5);//10); // probably 0 not allowed
	matrix->CurrentDACConfig->AddParameter("slowdownend", "3,2,1,0",  2);//1);
	matrix->CurrentDACConfig->AddParameter("timerend", "3,2,1,0",  1);//8); // darf nicht 0!! sonst werden debug ausgaben verschluckt
	matrix->CurrentDACConfig->AddParameter("ckdivend2", "5,4,3,2,1,0",  4);//1);
	matrix->CurrentDACConfig->AddParameter("ckdivend", "5,4,3,2,1,0",  4);//1);
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


	// Sync RO state machine ckdivend with the one in the chip
	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);
	 *ro=(*ro & 0xFFFFFF00)+(matrix->CurrentDACConfig->GetParameter("ckdivend") & 0xFF);

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

//	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
//
//	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
//	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
//	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
//	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
//	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);
//
//
//	 *ro = 0x20000;
//	 *fifo_config = 0b11;

	 *inj_flag = 0x1;
	 usleep(pulse_width);
	 *inj_flag = 0x0;

//	 sleep(1);
//	 *ro = 0x20000;
//	 *fifo_config = 0b10;

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

	this->writePixelInj(this->theMatrix,col,row,ana_state,hb_state);

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
		matrix->MatrixDACConfig->SetParameter("RamUp"+s, matrix->TDAC[col][row]); //0b1011
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
		matrix->MatrixDACConfig->SetParameter("RamDown"+s, matrix->TDAC[col][row]); //0b1011
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
	  this->setPulse(theMatrix,npulse,tup,tdown,amplitude);
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
			//matrix->TDAC[col][row] = (actual_value) ;
			}
		}
}

void ATLASPix::setOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	uint32_t actual_value = value;
	if(value>7){
		std::cout << "Value out of range, setting TDAC to 7" << std::endl;
		actual_value = 7;
	}
	//matrix->MASK[col][row]=0;
	matrix->TDAC[col][row] = (actual_value << 1) | matrix->MASK[col][row];
	//std::cout << std::bitset<4>(matrix->TDAC[col][row]) << std::endl;

}

void ATLASPix::setMaskPixel(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	matrix->MASK[col][row]=value;
	matrix->TDAC[col][row] = matrix->TDAC[col][row] << 1  | matrix->MASK[col][row];
	//std::cout << std::bitset<4>(matrix->TDAC[col][row]) << std::endl;
}


void ATLASPix::MaskPixel(uint32_t col,uint32_t row){

	this->setMaskPixel(theMatrix,col,row,1);
	//std::cout << "pixel masked col:" << col << " row: " << row << " " << theMatrix->MASK[col][row] << std::endl;
	this->writeOneTDAC(theMatrix,col,row,0);
	this->SetPixelInjection(col,row,0,0);
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
			matrix->MatrixDACConfig->SetParameter("RamDown"+s,matrix->TDAC[col][row]); //0b1011
			//std::cout << std::bitset<4>(matrix->TDAC[col][row]) << std::endl;

			//matrix->MatrixDACConfig->SetParameter("RamUp"+s, 4, ATLASPix_Config::LSBFirst,  matrix->TDAC[col][row]); //0b1011
			}
			else{
			//matrix->MatrixDACConfig->SetParameter("RamDown"+s, 4, ATLASPix_Config::LSBFirst,  matrix->TDAC[col][row]); //0b1011
			matrix->MatrixDACConfig->SetParameter("RamUp"+s,matrix->TDAC[col][row]); //0b1011
			//std::cout << std::bitset<4>(matrix->TDAC[col][row]) << std::endl;

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

	this->writeUniformTDAC(theMatrix,value);

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


			this->setOneTDAC(theMatrix,col,row,TDAC);
			this->setMaskPixel(theMatrix,col,row,mask);

			//std::cout  << " col: " << col << " row: " << row << " TDAC: " << this->simpleM1->TDAC[col][row] << " mask : " <<  this->simpleM1->MASK[col][row] << std::endl;
			//std::cout  << " ---------------------------- " << std::endl;

		}
		this->writeAllTDAC(theMatrix);

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


	this->SetPixelInjection(theMatrix,col,row,1,1);
	this->resetCounters();

	int cnt=0;

	double vinj=vmin;
	double dv = (vmax-vmin)/(npoints-1);

	for(int i=0;i<npoints;i++){
		this->pulse(npulses,10000,10000,vinj);
		cnt=this->readCounter(theMatrix);
		std::cout << "V : " << vinj << " count : " << cnt << std::endl;
		vinj+=dv;
	}

	this->SetPixelInjection(col,row,0,0);

}



void ATLASPix::SetInjectionMask(uint32_t mask,uint32_t state){


	for (int col =0 ;col<theMatrix->ncol;col++){
		int row=0;
		if((col+mask)%5){

		if(theMatrix->type==1){
			std::string s = to_string(col);

			if(row<200){
			theMatrix->MatrixDACConfig->SetParameter("RamDown"+s, theMatrix->TDAC[col][row]); //0b1011
			theMatrix->MatrixDACConfig->SetParameter("RamUp"+s, theMatrix->TDAC[col][row]); //0b1011
			theMatrix->MatrixDACConfig->SetParameter("colinjDown"+s,  state);
			theMatrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
			theMatrix->MatrixDACConfig->SetParameter("unusedDown"+s,  3);
			theMatrix->MatrixDACConfig->SetParameter("colinjUp"+s,   0);
			theMatrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
			theMatrix->MatrixDACConfig->SetParameter("unusedUp"+s,  3);

			}
			else{
			//std::cout << "up pixels" << std::endl;
			theMatrix->MatrixDACConfig->SetParameter("RamUp"+s,theMatrix->TDAC[col][row]); //0b1011
			theMatrix->MatrixDACConfig->SetParameter("RamDown"+s, theMatrix->TDAC[col][row]); //0b1011
			theMatrix->MatrixDACConfig->SetParameter("colinjDown"+s,  0);
			theMatrix->MatrixDACConfig->SetParameter("hitbusDown"+s,  0);
			theMatrix->MatrixDACConfig->SetParameter("unusedDown"+s,  3);
			theMatrix->MatrixDACConfig->SetParameter("colinjUp"+s,   state);
			theMatrix->MatrixDACConfig->SetParameter("hitbusUp"+s,  0);
			theMatrix->MatrixDACConfig->SetParameter("unusedUp"+s,  3);


			}

		}
		else{

			int double_col=int(std::floor(double(col)/2));
			std::string col_s = to_string(double_col);
			if(col%2==0){
					theMatrix->MatrixDACConfig->SetParameter("RamL"+col_s,theMatrix->TDAC[col][row] & 0b111);
					theMatrix->MatrixDACConfig->SetParameter("colinjL"+col_s,state);
			}
			else {
					theMatrix->MatrixDACConfig->SetParameter("RamR"+col_s, theMatrix->TDAC[col][row] & 0b111);
					theMatrix->MatrixDACConfig->SetParameter("colinjR"+col_s,state);
			}


		}

	}};

	for (int row =0 ;row<theMatrix->nrow;row++){
		int col=0;
		if((row+mask)%25){

			std::string row_s = to_string(row);
			theMatrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
			theMatrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
			theMatrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,state);
			theMatrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);
	}};

	this->ProgramSR(theMatrix);

	for (int row =0 ;row<theMatrix->nrow;row++){
		int col=0;
		if((row+mask)%5){

			std::string row_s = to_string(row);
			theMatrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);
		};
	};

	this->ProgramSR(theMatrix);


}


void ATLASPix::isLocked(){
	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);

	 if((*fifo_status >> 5) & 0b1){
		 std::cout << "yes" << std::endl;
	 }
	 else{
		 std::cout << "no" << std::endl;
	 }


}



pearydata ATLASPix::getData(){

	const unsigned int colmask = 0b11111111000000000000000000000000;
	const unsigned int ts2mask = 0b00000000111111000000000000000000;
	const unsigned int ts1mask = 0b00000000000000111111111100000000;
	const unsigned int rowmask = 0b00000000000000000000000011111111;



	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);


	 uint64_t d1=0;
	 uint64_t d2=0;

	 std::ofstream disk;
	 disk.open("PEARYDATA/tmp.dat",std::ios::out);
	 disk << "X:	Y:	   TS1:	   TS2:		FPGA_TS:   SyncedCNT:   TR_CNT:	ATPBinCounter:   ATPGreyCounter:	" << std::endl;

	 Color::Modifier red(Color::FG_RED);
	 Color::Modifier green(Color::FG_GREEN);
	 Color::Modifier blue(Color::FG_BLUE);
	 Color::Modifier cyan(Color::FG_CYAN);
	 Color::Modifier mag(Color::FG_MAGENTA);
	 Color::Modifier bold(Color::BOLD);
	 Color::Modifier rev(Color::REVERSE);
	 Color::Modifier def(Color::FG_DEFAULT);
	 Color::Modifier reset(Color::RESET);



	 uint64_t dataw =0;

	 uint64_t fpga_ts =0;
	 uint64_t fpga_ts_prev =0;

	 uint32_t hit =0;
	 uint32_t timestamp =0;

	 uint32_t ATPSyncedCNT,TrCNT;

	 int to,cnt;
	 int prev_tr=0;
	 to=1;
	 cnt=0;

	 this->setSpecialRegister("trigger_mode",2);

	 *fifo_config = 0b11;




	 while(to){
		 while((*fifo_status & 0x4)==0 & to==1){
			 //usleep(1);
			 cnt++;
			 if (cnt>1e7){to=0;}
			 };

		 if(to==0){break;}

		 d1 = *data;
		 while((*fifo_status & 0x1)==0){continue;};
		 d2 = *data;
		 //usleep(10);

		 dataw = (d2 << 32) | (d1);
		 uint32_t stateA = dataw>>56 & 0xFF;
		 //std::cout  << blue << bold << rev << dataw << " " << stateA <<   std::endl;


		 if(stateA==0) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<24;
			 //std::cout  << blue << bold << rev  << "stateA : " << stateA << reset  << std::endl;
			 //std::cout <<  bold <<   "DO: " << DO << " TrTS: "  <<TrTS << " TSf: " <<  TSf << reset << std::endl;
		 }
		 else if(stateA==1) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<16;
			 //std::cout  << blue << bold << rev  << "stateA : " << stateA << reset  << std::endl;
			 //std::cout <<  bold <<   "DO: " << DO << " TrTS: "  <<TrTS << " TSf: " <<  TSf << reset << std::endl;
		 }
		 else if(stateA==2) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<8;
			 //std::cout  << blue << bold << rev  << "stateA : " << stateA << reset  << std::endl;
			 //std::cout <<  bold <<   "DO: " << DO << " TrTS: "  <<TrTS << " TSf: " <<  TSf << reset << std::endl;
		 }
		 else if(stateA==3) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO;
			 //std::cout  << blue << bold << rev  << "stateA : " << stateA << reset  << std::endl;
			 //std::cout <<  bold <<   "DO: " << DO << " TrTS: "  <<TrTS << " TSf: " <<  TSf << reset << std::endl;
			 //disk << std::bitset<32>(timestamp) << " " << std::dec << (timestamp >>8) << " " << gray_decode(timestamp & 0xF) << std::endl;
		 }

		 else if(stateA==4) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO<<24;
			 //TrTS1 = (dataw>>8) & 0xFFFFFF;
			 //TSf = (dataw) & 0b111111;
			 fpga_ts = 0;
			 fpga_ts += (dataw << 32);

			 //std::cout  << blue << bold << rev  << "dataw : " << std::bitset<64>(dataw) << reset  << std::endl;
			 //std::cout <<  bold <<   "DO: " << DO << " TrTS: "  <<TrTS << " TSf: " <<  TSf << reset << std::endl;
		 }
		 else if(stateA==5) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO << 16;
			 TrCNT = (dataw) & 0xFFFFFFFF;
			 //std::cout  << cyan << bold << rev <<  "stateA : " << stateA << reset  << std::endl;
			 //std::cout <<  bold << "DO: " << DO  << " TrTS: "<< TrTS << " TrCnt: " << TrCnt << reset << std::endl;

		 }
		 else if(stateA==6){
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO << 8;
			 ATPSyncedCNT = dataw & 0xFFFFFF;
			 //std::cout << green << bold << rev << "dataw : " << std::bitset<64>(dataw)   << reset << std::endl;
			 //std::cout << bold  << "DO: " << DO << " TS: " <<  TS << " TOT: " << TOT << reset << std::endl;
		 }
		 else if(stateA==7) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit+=DO ;
			 fpga_ts+=(dataw & 0x00000000FFFFFFFF) ;
			 pixelhit pix=decodeHit(hit,timestamp,fpga_ts,ATPSyncedCNT,TrCNT);

			 //Write hit to disk
			 disk << pix.col  << "	" << pix.row << "	" << pix.ts1 << "	" << pix.ts2
					 << "	" << pix.fpga_ts << "	" << pix.SyncedTS  << " " << pix.triggercnt
					 << " " << pix.ATPbinaryCnt << " " << pix.ATPGreyCnt << std::endl;


			 std::cout << green << "Event : " << TrCNT << std::endl;
			 //double delay=double(fpga_ts-fpga_ts_prev)*(10.0e-9);
			 //std::cout << red << bold << rev << "ts : " <<fpga_ts   << reset << std::endl;
			 //std::cout <<  rev << bold << red << "FPGA TS: "<<fpga_ts << " tr CNT : " << TrCNT << " delay : " << delay <<  reset << std::endl;

			 std::cout << bold  << rev<< blue << "X: " << pix.col  << " Y: " << pix.row << " TS1: " << pix.ts1 << " TS2: " << pix.ts2
					 << " FPGATS: " << pix.fpga_ts << " SyncedTS: " << pix.SyncedTS  << " TriggerCNT: " << pix.triggercnt
					 << " ATPBinCNT: " << pix.ATPbinaryCnt << " ATPGreyCNT: " << pix.ATPGreyCnt  << reset << std::endl;
			 fpga_ts_prev=fpga_ts;
			 hit=0;
		 }
		 else  {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 //std::cout  << red << "stateA : " << stateA << std::endl;
			 //std::cout <<  bold << "DO: "<< std::bitset<8>(DO) << reset << std::endl;

		 };

	 }//while data

//		 this->SetPixelInjection(col,row,0,0);
//
//		 }};


	 //*fifo_config = 0b10;
	 disk.close();

	 pearydata dummy;
	 return dummy;

}

std::vector<int> ATLASPix::getCountingData(){

	 const unsigned int colmask = 0b11111111000000000000000000000000;
	 const unsigned int ts2mask = 0b00000000111111000000000000000000;
	 const unsigned int ts1mask = 0b00000000000000111111111100000000;
	 const unsigned int rowmask = 0b00000000000000000000000011111111;

	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

	 uint64_t d1=0;
	 uint64_t d2=0;
	 uint64_t dataw =0;
	 uint64_t fpga_ts =0;
	 uint64_t fpga_ts_prev =0;

	 uint32_t row,col,ts1,ts2;

	 //std::ofstream disk;
	 //disk.open("PEARYDATA/tmp.dat",std::ios::out);
	 //disk << "X:	Y:	TS1:	TS2:	FPGA_TS:	TR_CNT:	TR_DELAY:	" << std::endl;

	 Color::Modifier red(Color::FG_RED);
	 Color::Modifier green(Color::FG_GREEN);
	 Color::Modifier blue(Color::FG_BLUE);
	 Color::Modifier cyan(Color::FG_CYAN);
	 Color::Modifier mag(Color::FG_MAGENTA);
	 Color::Modifier bold(Color::BOLD);
	 Color::Modifier rev(Color::REVERSE);
	 Color::Modifier def(Color::FG_DEFAULT);
	 Color::Modifier reset(Color::RESET);
	 uint32_t hit =0;
	 uint32_t timestamp =0;
	 uint32_t chipts =0;

	 uint32_t TrTS,TrTS1,TSf,TrTS2,TrCNT;

	 int to,cnt;
	 int prev_tr=0;
	 to=1;
	 cnt=0;

	 //READOUT on
	 *fifo_config = 0b0;
	 *ro = 0x00000;
	 usleep(1);
	 *fifo_config = 0b100000;
	 *ro = 0xF0000;
	 usleep(10);
	 *ro = 0x00000;
	 *fifo_config = 0b11;

	 std::vector<int> pixels (25*400,0);

	 while(to){
		 while((*fifo_status & 0x4)==0 & to==1){
			 usleep(1);
			 cnt++;
			 if (cnt>1e7){to=0;}
			 };

		 if(to==0){break;}

		 d1 = *data;
		 usleep(10);
		 d2 = *data;
		 usleep(10);

		 dataw = (d2 << 32) | (d1);
		 uint32_t stateA = dataw>>56 & 0xFF;

		 if(stateA==0) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<24;
		 }
		 else if(stateA==1) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<16;
		 }
		 else if(stateA==2) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO<<8;
		 }
		 else if(stateA==3) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 timestamp += DO;
		 }
		 else if(stateA==4) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO<<24;
			 TrTS1 = (dataw>>8) & 0xFFFFFF;
			 TSf = (dataw) & 0b111111;
			 fpga_ts = 0;
			 fpga_ts += (dataw << 32);
		 }
		 else if(stateA==5) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO << 16;
			 TrTS2 = (dataw>>32) & 0xFFFF;
			 TrCNT = (dataw) & 0xFFFFFFFF;
		 }
		 else if(stateA==6){
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit += DO << 8;
			 chipts= ((dataw & 0xFFF00)>>2) | (dataw & 0b111111);
		 }
		 else if(stateA==7) {
			 uint32_t DO = (dataw >> 48) & 0xFF;
			 hit+=DO ;
//			 pixelhit pix=decodeHit(hit);
			 fpga_ts+=(dataw & 0x00000000FFFFFFFF) ;
			 //std::cout << rev << bold << red << "X: "<< pix.col  << " Y: " << pix.row << " " << pix.ts1 << " " << pix.ts2 << reset << std::endl;
			 //disk << pix.col  << "	" << pix.row << "	" << pix.ts1 << "	" << pix.ts2 << "	" << fpga_ts << "	" << TrCNT << std::endl;

			 double delay=double(fpga_ts-fpga_ts_prev)*(10.0e-9);
			 //std::cout << rev << bold << red << "FPGA TS: "<<fpga_ts << " tr CNT : " << TrCNT << " delay : " << delay <<  reset << std::endl;
			 fpga_ts_prev=fpga_ts;
			 hit=0;

//			 pixels.at(pix.col + pix.row*25) = pixels.at(pix.col + pix.row*25) +1;
;

		 }
		 else  {
			 uint32_t DO = (dataw >> 48) & 0xFF;
		 };
	 }
	 //disk.close();
}

//void ATLASPix::daqStart(){
//
//	//std::thread t(&caribou::ATLASPix::TakeData);
//
//
//}



void ATLASPix::TakeData(){

	const unsigned int colmask = 0b11111111000000000000000000000000;
	const unsigned int ts2mask = 0b00000000111111000000000000000000;
	const unsigned int ts1mask = 0b00000000000000111111111100000000;
	const unsigned int rowmask = 0b00000000000000000000000011111111;



	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);

	 volatile uint32_t* data = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x0);
	 volatile uint32_t* fifo_status = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x4);
	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);
	 volatile uint32_t* leds = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0xC);
	 volatile uint32_t* ro = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x10);

	 *fifo_config = 0b1;
	 *ro = 0xF0000;

	 uint64_t d1=0;
	 uint64_t d2=0;
	 uint64_t dataw =0;

	 uint32_t row,col,ts1,ts2;

	 std::ofstream disk;
	 disk.open("PEARYDATA/tmp.dat",std::ios::out | std::ios::binary);

	 Color::Modifier red(Color::FG_RED);
	 Color::Modifier green(Color::FG_GREEN);
	 Color::Modifier blue(Color::FG_BLUE);
	 Color::Modifier cyan(Color::FG_CYAN);
	 Color::Modifier mag(Color::FG_MAGENTA);
	 Color::Modifier bold(Color::BOLD);
	 Color::Modifier rev(Color::REVERSE);
	 Color::Modifier def(Color::FG_DEFAULT);
	 Color::Modifier reset(Color::RESET);

	 for(int i=0;i<10000;i++){
//	 while(1){
		 while((*fifo_status & 0x1)==1){usleep(1);}

//		 std::cout << "fifo_status" <<   std::bitset<32>(*fifo_status) << std::endl;
		 d1 = *data;
//		 std::cout << "data word 1: " <<   std::bitset<32>(d1) << std::endl;

//		 std::cout << "fifo_status: " <<   std::bitset<32>(*fifo_status) << std::endl;
		 d2 = *data;

//		 std::cout << "data word 2: " <<   std::bitset<32>(d2) << std::endl;
//		 //std::cout << "data word 2: " << std::hex <<   d2 << std::endl;
//
		 dataw = (d2 << 32) | (d1);
		 uint32_t stateA = dataw>>56 & 0xFF;

		 if(stateA==6){std::cout << green << bold << rev << "stateA : " << stateA  << std::endl;}
		 else if(stateA==4) {std::cout  << blue << "stateA : " << stateA  << std::endl;}
		 else if(stateA==5) {std::cout  << cyan << "stateA : " << stateA  << std::endl;}
		 else if(stateA==7) {std::cout  << mag << "stateA : " << stateA  << std::endl;}
		 else  {std::cout  << red << "stateA : " << stateA << std::endl;};


		 std::cout << "data word: " <<   std::bitset<64>(dataw) << std::endl;


		 if(stateA==6){
////
		 uint32_t hit = ((dataw>>48) & 0xFF) << 24 | (dataw & 0xFFFFFF);
		 col = (hit & colmask) >> 24;
		 ts2 = (hit & ts2mask) >> 18;
		 ts1 = (hit & ts1mask) >> 8;
		 row = (hit & rowmask);

		 std::cout  << "Hit data : "<< std::bitset<8>(col) << " "  << std::bitset<8>(row) <<  " " <<  std::bitset<10>(ts1) << " " <<   std::bitset<6>(ts2)   << reset <<   std::endl;
		 std::cout  << std::dec  << "col : " << col << " row : " << row << " TS1 : " << ts1 << " TS2 : " << ts2<< std::endl;

		 disk << hit;

		 }

		 //std::cout  << std::endl;
//
		// usleep(1000);

	 }

	 disk.close();

}

void ATLASPix::dataTuning(ATLASPixMatrix *matrix, double vmax,int nstep, int npulses) {
	LOG(logINFO) << "Tuning using data" << DEVICE_NAME;

	int spacing_row = 25;
	int spacing_col = 5;

	std::vector<std::vector<int>> pixels(nstep, std::vector< int >( 25*400 ));
	int voltage_step = 0;

	for (int TDAC_value = 0; TDAC_value < 8; TDAC_value++){
		initTDAC(matrix, TDAC_value);
		writeAllTDAC(matrix);
		for (unsigned int selrow = 0; selrow < spacing_row; ++selrow) {
			for (unsigned int selcol = 0; selcol < spacing_col; ++selcol) {
				for (unsigned int col = 0; col < matrix->ncol; ++col) {
					for (unsigned int row = 0; row < matrix->nrow; ++row) {
						if ((row % spacing_row == selrow) && (col % spacing_col == selcol)) {
							this->SetPixelInjection(col,row,1,1);
						} else {
							this->SetPixelInjection(col,row,0,0);
						}
					}
				}

				for(double v=0;v<=vmax;v+=(vmax/nstep)){
					setPulse(matrix,npulses,100,100,v);
					sendPulse();
					//pixels[voltage_step] = this->getCountingData();
					pixels[voltage_step] = this->getCountingData();
					voltage_step++;
				}
			}
		}

		int col, row;
		for (unsigned int index = 0; index < matrix->ncol * matrix->nrow; ++index) {
			col = index%25;
			row = (index - col)/25;
			std::cout << "Index:    " << index << " X:      " << col << "   Y:      " << row << "   ";
			for (int volt = 0; volt < nstep; volt++) {
				std::cout << pixels[volt][index] << "   ";
			}
			std::cout << std::endl << std::endl;
		}

	}

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
	ss << "PEARYDATA/ATLASPixGradeA_02/" <<"_"<< ptm->tm_year+1900 <<"_"<< ptm->tm_mon+1 <<"_"<< ptm->tm_mday <<"@"<< ptm->tm_hour+1 <<"_"<< ptm->tm_min+1 << "_"<<ptm->tm_sec+1 <<"_VNPix";

	std::string cmd;
	cmd+="mkdir -p ";
	cmd+= " ";
	cmd+=ss.str();
	const int dir_err = system(cmd.c_str());

	std::string filename;
	filename+=ss.str();
	filename+="/";
	filename+="M1_VNDAC_";
	filename+=std::to_string(theMatrix->CurrentDACConfig->GetParameter("VNDACPix"));
	filename+="_TDAC_";
	filename+=std::to_string(theMatrix->TDAC[0][0]>>1);
	//filename+=ss.str();
	filename+=".txt";


	std::cout << "writing to file : " << filename << std::endl;


	std::ofstream myfile;
	myfile.open (filename);

	myfile << npoints << std::endl;

    std::clock_t start;
    double duration;

	for(int col=0;col< theMatrix->ncol; col++){
		for(int row=0;row< theMatrix->nrow; row++){



		    //start = std::clock();

			if(row%5==0){std::cout << "X: " << col << " Y: " << row << "\n" ;}


			vinj=vmin;
			//this->SetPixelInjection(theMatrix,0,0,1,1);
			//this->SetPixelInjection(theMatrix,0,0,0,0);

			this->SetPixelInjection(theMatrix,col,row,1,1);
			this->resetCounters();

			for(int i=0;i<npoints;i++){
				this->pulse(npulses,1000,1000,vinj);
				//cnt=this->readCounter(theMatrixISO);
				cnt=this->readCounter(theMatrix);
				//this->getData();
				myfile << vinj << " " << cnt << " ";
				//std::cout << "V : " << vinj << " count : " << cnt << std::endl;
				vinj+=dv;
			}

			this->SetPixelInjection(theMatrix,col,row,0,0);
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
	filename+=std::to_string(theMatrix->CurrentDACConfig->GetParameter("VNDACPix"));
	filename+="_TDAC_";
	filename+=std::to_string(theMatrix->TDAC[0][0]>>1);
	//filename+=ss.str();
	filename+=".txt";


	std::cout << "writing to file : " << filename << std::endl;


	std::ofstream myfile;
	myfile.open (filename);

	myfile << npoints << std::endl;

    std::clock_t start;
    double duration;

	for(int col=0;col< theMatrix->ncol; col++){
		for(int row=0;row< theMatrix->nrow; row++){



		    //start = std::clock();

			if(row%5==0){std::cout << "X: " << col << " Y: " << row << "\n" ;}


			vinj=vmin;
			//this->SetPixelInjection(theMatrix,0,0,1,1);
			//this->SetPixelInjection(theMatrix,0,0,0,0);

			this->SetPixelInjection(theMatrix,col,row,1,1);
			this->resetCounters();

			for(int i=0;i<npoints;i++){
				this->pulse(npulses,1000,1000,vinj);
				//cnt=this->readCounter(theMatrixISO);
				cnt=this->readCounter(theMatrix);
				//this->getData();
				myfile << vinj << " " << cnt << " ";
				//std::cout << "V : " << vinj << " count : " << cnt << std::endl;
				vinj+=dv;
			}

			this->SetPixelInjection(theMatrix,col,row,0,0);
			myfile << std::endl;

			//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
			//std::cout << duration << " s \n" ;


		}}

	myfile.close();

}



void ATLASPix::TDACScan(std::string basefolder,int VNDAC,int step,double vmin,double vmax,uint32_t npulses,uint32_t npoints){


	this->WriteConfig(basefolder+"/config");

	theMatrix->CurrentDACConfig->SetParameter("VNDACPix",VNDAC);


	for(int tdac=0;tdac<=7;tdac+=step){

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
		std::cout << "Count  : " << this->readCounter(theMatrix)<< std::endl;
		cnt = this->readCounter(theMatrix);
		threshold -=0.001;


	}

	std::cout << "noise floor at : " << threshold+0.001 << std::endl;

}


//CaR Board related


void ATLASPix::reset() {
  //LOG(logINFO) << "Resetting " << DEVICE_NAME;

	 void* readout_base = _hal->getMappedMemoryRW(ATLASPix_READOUT_BASE_ADDRESS, ATLASPix_READOUT_MAP_SIZE, ATLASPix_READOUT_MASK);
	 volatile uint32_t* fifo_config = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(readout_base) + 0x8);

	 *fifo_config = 0b10000;
	 sleep(1);
	 *fifo_config = 0b00000;

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

  this->setVoltage("VDDD",ATLASPix_VDDD,ATLASPix_VDDD_CURRENT);
  this->switchOn("VDDD");

  this->setVoltage("VDDA",ATLASPix_VDDA,ATLASPix_VDDA_CURRENT);
  this->switchOn("VDDA");

  this->setVoltage("VSSA",ATLASPix_VSSA,ATLASPix_VSSA_CURRENT);
  this->switchOn("VSSA");



  // Analog biases

  this->setVoltage("GNDDACPix",theMatrix->GNDDACPix);
  this->switchOn("GNDDACPix");

  this->setVoltage("VMinusPix",theMatrix->VMINUSPix);
  this->switchOn("VMinusPix");

  this->setVoltage("GatePix",theMatrix->GatePix);
  this->switchOn("GatePix");



  // Threshold and Baseline
  this->setVoltage("ThPix",theMatrix->ThPix);
  this->switchOn("ThPix");

  this->setVoltage("BLPix",theMatrix->BLPix);
  this->switchOn("BLPix");




}

void ATLASPix::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off ATLASPix";

  LOG(logDEBUG) << "Powering off VDDA";
  this->switchOff("VDDA");

  LOG(logDEBUG) << "Powering off VDDD";
  this->switchOff("VDDD");

  LOG(logDEBUG) << "Powering off VSSA";
  this->switchOff("VSSA");


  LOG(logDEBUG) << "Turning off GNDDacPix";
  this->switchOff("GNDDACPix");

  LOG(logDEBUG) << "Turning off VMinusPix_M1";
  this->switchOff("VMinusPix");

  LOG(logDEBUG) << "Turning off GatePix_M1";
  this->switchOff("GatePix");
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


	std::ofstream myfile;
	myfile.open(basename + ".cfg" );


	static std::vector<std::string> VoltageDACs;
	VoltageDACs = {"BLPix", "nu2","ThPix","nu3"};

	for(auto const& value: VoltageDACs) {
		myfile << std::left << std::setw(20) << value << " " << theMatrix->VoltageDACConfig->GetParameter(value) << std::endl;
	}

	static std::vector<std::string> CurrentDACs;
	CurrentDACs = {"unlock","BLResPix","ThResPix","VNPix","VNFBPix","VNFollPix","VNRegCasc","VDel","VPComp","VPDAC","VNPix2","BLResDig","VNBiasPix","VPLoadPix","VNOutPix",
					"VPVCO","VNVCO","VPDelDclMux","VNDelDclMux","VPDelDcl","VNDelDcl","VPDelPreEmp","VNDelPreEmp","VPDcl","VNDcl","VNLVDS","VNLVDSDel","VPPump","nu",
					"RO_res_n","Ser_res_n","Aur_res_n","sendcnt","resetckdivend","maxcycend","slowdownend","timerend","ckdivend2","ckdivend","VPRegCasc","VPRamp","VNcompPix",
					"VPFoll","VPFoll","VNDACPix","VPBiasRec","VNBiasRec","Invert","SelEx","SelSlow","EnPLL","TriggerDelay","Reset","ConnRes","SelTest","SelTestOut"};


	for(auto const& value: CurrentDACs) {
		myfile << std::left << std::setw(20) << value << " " << theMatrix->CurrentDACConfig->GetParameter(value) << std::endl;
	}


	static std::vector<std::string> ExternalBias;
	ExternalBias = {"BLPix_ext", "ThPix_ext","VMINUSPix","GNDDACPix","GatePix"};

	myfile << std::left << std::setw(20)<< "ThPix_ext" << " " << theMatrix->ThPix << std::endl;
	myfile << std::left << std::setw(20)<< "BLPix_ext"<< " " << theMatrix->BLPix << std::endl;
	myfile << std::left << std::setw(20)<< "VMINUSPix" << " " << theMatrix->VMINUSPix << std::endl;
	myfile << std::left << std::setw(20)<< "GNDDACPix" << " " << theMatrix->GNDDACPix << std::endl;
	myfile << std::left << std::setw(20)<< "GatePix" << " " << theMatrix->GatePix << std::endl;



	std::ofstream TDACFile;
	TDACFile.open(basename + "_TDAC.cfg" );


	for(int col = 0;col<theMatrix->ncol;col++){
		for(int row = 0;row<theMatrix->nrow;row++){

			TDACFile << std::left << std::setw(3) << col << " " << std::left << std::setw(3) << row << " "  << std::left << std::setw(2) << theMatrix->TDAC[col][row] << " " << std::left << std::setw(1) << theMatrix->MASK[col][row] << std::endl ;
		}
	}


	myfile.close();
	TDACFile.close();
}



void ATLASPix::LoadConfig(std::string basename){

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
			theMatrix->CurrentDACConfig->SetParameter(reg,value);
		}

		else if(std::find(VoltageDACs.begin(), VoltageDACs.end(), reg) != VoltageDACs.end()){
			configfile >> value;
			theMatrix->VoltageDACConfig->SetParameter(reg,value);
		}

		else {
			std::cout << "unknown register : " << reg << std::endl;
		}

	}

	this->ProgramSR(theMatrix);

	this->LoadTDAC(basename+"_TDAC.cfg");



}


caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  ATLASPix* mDevice = new ATLASPix(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
