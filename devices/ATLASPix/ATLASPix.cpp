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

  //Data structure containing the info about the matrices
  simpleM1 = new ATLASPixMatrix();
  simpleM1ISO = new ATLASPixMatrix();
  simpleM2 = new ATLASPixMatrix();


  simpleM1->ncol=ncol_m1;
  simpleM1ISO->ncol=ncol_m1iso;
  simpleM2->ncol=ncol_m2;

  simpleM1->ndoublecol=ncol_m1/2;
  simpleM1ISO->ndoublecol=ncol_m1iso/2;
  simpleM2->ndoublecol=ncol_m2/2;

  simpleM1->nrow=nrow_m1;
  simpleM1ISO->nrow=nrow_m1iso;
  simpleM2->nrow=nrow_m2;

  simpleM1->counter=1;
  simpleM1ISO->counter=2;
  simpleM2->counter=3;

  simpleM1->nSRbuffer = 99;
  simpleM1ISO->nSRbuffer = 99;
  simpleM2->nSRbuffer = 84;

  simpleM1->extraBits = 8;
  simpleM1ISO->extraBits = 8;
  simpleM2->extraBits = 8;

  simpleM1->SRmask=0x1;
  simpleM1->SRmask=0x2;
  simpleM1->SRmask=0x4;


  //Configuring the clock to 160 MHz
  LOG(logINFO) << "Setting clock to 160MHz " << DEVICE_NAME;
  configureClock();




  this->initTDAC(simpleM1,4);
  this->initTDAC(simpleM1ISO,4);
  this->initTDAC(simpleM2,4);

  this->Initialize_SR(simpleM1);
  this->Initialize_SR(simpleM1ISO);
  this->Initialize_SR(simpleM2);



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

 this->powerOn();
 usleep(1000);


 // Build the SR string with default values and shift in the values in the chip
  this->Fill_SR(simpleM1);
  this->Shift_SR(simpleM1);
  this->Fill_SR(simpleM1ISO);
  this->Shift_SR(simpleM1ISO);
  this->Fill_SR(simpleM2);
  this->Shift_SR(simpleM2);

  //this->ComputeSCurves(0,0.5,50,128,100,100);



//  this->resetCounters();
//  this->setPulse(100,1,1, 0.8);
//
//
//  while(1){
//	  std::cout << "sending pulse" << std::endl;
//	  this->sendPulse();
//	  usleep(2000);
//	  std::cout << "Counter 0 : " << this->readCounter(0) << std::endl;
//	  std::cout << "Counter 1 : " << this->readCounter(1) << std::endl;
//	  std::cout << "Counter 2 : " << this->readCounter(2) << std::endl;
//	  std::cout << "Counter 3 : " << this->readCounter(3) << std::endl;
//	  //int ddd = 0;
//	  //std::cin >> ddd;
//	 ///if (ddd=1){this->resetCounters();}
//  }



 // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}


void ATLASPix::initTDAC(ATLASPixMatrix *matrix,uint32_t value){

	for(int col=0;col<matrix->ncol;col++){
		for(int row=0;row < matrix->nrow;row++){
			matrix->TDAC[col][row] = value;
			}
		}

//
//	switch(matrix){
//					case 0 :
//					{
//						for(int col=0;col<ncol_m1;col++){
//							for(int row=0;row<nrow_m1;row++){
//								this->M1_TDAC[col][row] = value;
//								}
//							}
//						break;
//					}
//
//
//					case 1:
//					{
//						for(int col=0;col<ncol_m1iso;col++){
//							for(int row=0;row<nrow_m1iso;row++){
//								this->M1ISO_TDAC[col][row] = value;
//								}
//							}
//						break;
//
//					}
//					case 2:
//					{
//						for(int col=0;col<ncol_m2;col++){
//							for(int row=0;row<nrow_m2;row++){
//								this->M2_TDAC[col][row] = value;
//								}
//							}
//						break;
//					}
//					default :
//						break;
//				}


}


void ATLASPix::setOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	matrix->TDAC[col][row] = value;
}



void ATLASPix::writeOneTDAC(ATLASPixMatrix *matrix,uint32_t col,uint32_t row,uint32_t value){

	std::string col_s = to_string(int(std::floor(double(col)/2)));
	if(col%2==0){

	matrix->MatrixDACConfig->SetParameter("RamL"+col_s, value);
	matrix->MatrixDACConfig->SetParameter("colinjL"+col_s,0);
	}
	else {
	matrix->MatrixDACConfig->SetParameter("RamR"+col_s, value);
	matrix->MatrixDACConfig->SetParameter("colinjR"+col_s,0);
	}

	std::string row_s = to_string(row);
	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

	this->Fill_SR(0);
	this->Shift_SR(0);

	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);

}

void ATLASPix::writeAllTDAC(ATLASPixMatrix *matrix){


	std::string col_s =0;
	int double_col=0;
    for (int row = 0; row < nrow_m1; row++){
    	for (int col = 0; col <ncol_m1; col++){
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

    	std::string row_s = to_string(row);
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,1);
    	matrix->MatrixDACConfig->SetParameter("unused"+row_s,  0);
    	matrix->MatrixDACConfig->SetParameter("rowinjection"+row_s,0);
    	matrix->MatrixDACConfig->SetParameter("analogbuffer"+row_s,0);

    	this->Fill_SR(0);
    	this->Shift_SR(0);
    	matrix->MatrixDACConfig->SetParameter("writedac"+row_s,0);


    }

}

//WIP
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

    for(double v=0;v<=vmax;v+=(vmax/nstep)){
		setPulse(npulses,tup,tdown,v);
		std::cout << "  bias :" << v << "V"<< std::endl;

		for(int col=0;col< matrix->ncol; col++){
			for(int row=0;row< matrix->nrow; row++){
				sendPulse();
				int sent = this->readCounter(0);
				int rec = this->readCounter(matrix);
				double ratio = double(rec)/sent;
				resetCounters();
				s_curves[v][col][row] = ratio;
			}
		}
	}
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	std::cout << "duration : " << duration << "s" << std::endl;

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
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
}


void ATLASPix::Initialize_SR(ATLASPixMatrix *matrix){

	matrix->CurrentDACConfig = new ATLASPix_Config();
	matrix->MatrixDACConfig = new ATLASPix_Config();
	matrix->VoltageDACConfig = new ATLASPix_Config();


	//DAC Block 1 for DIgital Part
	//AnalogDACs
	matrix->CurrentDACConfig->AddParameter("unlock",    4, ATLASPix_Config::LSBFirst, 0x101); // unlock = x101
	matrix->CurrentDACConfig->AddParameter("BLResPix", "5,4,3,1,0,2",  5);
	matrix->CurrentDACConfig->AddParameter("ThResPix", "5,4,3,1,0,2",  0);
	matrix->CurrentDACConfig->AddParameter("VNPix", "5,4,3,1,0,2",  20);
	matrix->CurrentDACConfig->AddParameter("VNFBPix", "5,4,3,1,0,2", 10);
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
	matrix->CurrentDACConfig->AddParameter("VNcompPix", "5,4,3,1,0,2",  0);     //VNComparator
	matrix->CurrentDACConfig->AddParameter("VPFoll", "5,4,3,1,0,2",  10);
	matrix->CurrentDACConfig->AddParameter("VNDACPix", "5,4,3,1,0,2",  0);
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



	//Column Register
	for (int col = 0; col <matrix->ndoublecol; col++)
	{

		std::string s = to_string(col);
		matrix->MatrixDACConfig->AddParameter("RamL"+s, 3, ATLASPix_Config::LSBFirst,  0);
		matrix->MatrixDACConfig->AddParameter("colinjL"+s, 1, ATLASPix_Config::LSBFirst,  0);
		matrix->MatrixDACConfig->AddParameter("RamR"+s, 3, ATLASPix_Config::LSBFirst,  0);
		matrix->MatrixDACConfig->AddParameter("colinjR"+s, 1, ATLASPix_Config::LSBFirst,  0);

	}


	//Row Register
	for (int row = 0; row < matrix->nrow; row++)
	{
		std::string s = to_string(row);
		matrix->MatrixDACConfig->AddParameter("writedac"+s, 1, ATLASPix_Config::LSBFirst, 0);
		matrix->MatrixDACConfig->AddParameter("unused"+s,   3, ATLASPix_Config::LSBFirst, 0);
		matrix->MatrixDACConfig->AddParameter("rowinjection"+s, 1, ATLASPix_Config::LSBFirst, 0);
		matrix->MatrixDACConfig->AddParameter("analogbuffer"+s, 1, ATLASPix_Config::LSBFirst, 0);
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




    matrix->Registers.clear();

    for (auto i = matrix->VoltageDACBits.begin(); i != matrix->VoltageDACBits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 matrix->Registers.push_back(buffer);
	 //std::cout << buffer << " ";
	 //this->printBits(sizeof(buffer),&buffer);
	 //std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;
       nbits++;
     }

   for (auto i = matrix->CurrentDACbits.begin(); i != matrix->CurrentDACbits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 matrix->Registers.push_back(buffer);
	 //std::cout << buffer << " ";
	 //this->printBits(sizeof(buffer),&buffer);
	 //std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;
       nbits++;

     }
   for (auto i = matrix->MatrixBits.begin(); i != matrix->MatrixBits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 matrix->Registers.push_back(buffer);
	 //std::cout << buffer << " ";
	 //this->printBits(sizeof(buffer),&buffer);
	 //std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;
       nbits++;

     }

   for (auto i = matrix->CurrentDACbits.begin(); i != matrix->CurrentDACbits.end(); ++i)
     {
       if(cnt==32){
	 cnt=0;
	 matrix->Registers.push_back(buffer);
	 //std::cout << buffer << " ";
	 //this->printBits(sizeof(buffer),&buffer);
	 //std::cout <<  std::endl;
	 buffer=0;
       };
       buffer += *i << cnt;
       cnt++;
       nbits++;
       //std::cout << cnt << std::endl;
     }

	 matrix->Registers.push_back(buffer);


     std::cout << "size of shift buffer " << matrix->Registers.size() << std::endl;
     std::cout << "number of bits " << nbits << std::endl;

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

	for(uint32_t i =0;i<matrix->nSRbuffer;i++){
		*RAM_address = i;
		*RAM_content = matrix->Registers[i];
		usleep(10);
		*RAM_write_enable =0x1;
		usleep(10);
		*RAM_write_enable =0x0;};

	*output_enable = matrix->SRmask;
	usleep(10);


	*Config_flag = 0x1;
	usleep(10000);
	*Config_flag = 0;
	*output_enable = 0x0;

}

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

void ATLASPix::setPulse(uint32_t npulse,uint32_t n_up,uint32_t n_down, double voltage){

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
	 *output_enable = 0xFFFFFFF;

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
