/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include "spi.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

std::string clicpix2::getName() { return DEVICE_NAME; }

void clicpix2::init() {
  LOG(logDEBUG) << "Initializing " << DEVICE_NAME;
  powerOn();
  configureClock();
}

clicpix2::~clicpix2() {
  
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

void clicpix2::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up CLICpix2";

  LOG(logDEBUG) << " CMLBUFFERS_VDD";
  _hal->setVoltageRegulator( PWR_OUT_4,_config.Get("cmlbuffers_vdd", CLICpix2_CMLBUFFERS_VDD),
			     _config.Get("cmlbuffers_vdd_current", CLICpix2_CMLBUFFERS_VDD_CURRENT) );
  _hal->powerVoltageRegulator( PWR_OUT_4, true );

  LOG(logDEBUG) << " CMLBUFFERS_VCCO";
  _hal->setVoltageRegulator( PWR_OUT_7,_config.Get("cmlbuffers_vcco", CLICpix2_CMLBUFFERS_VCCO),
			     _config.Get("cmlbuffers_vcco_current", CLICpix2_CMLBUFFERS_VCCO_CURRENT) );
  _hal->powerVoltageRegulator( PWR_OUT_7, true );
  
  LOG(logDEBUG) << " VDDCML";
  _hal->setVoltageRegulator( PWR_OUT_5,_config.Get("vdddcml",CLICpix2_VDDCML),
			     _config.Get("vdddcml_current", CLICpix2_VDDCML_CURRENT) );
  _hal->powerVoltageRegulator( PWR_OUT_5, true );
  
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator( PWR_OUT_1,_config.Get("vddd",CLICpix2_VDDD),
			     _config.Get("vddd_current", CLICpix2_VDDD_CURRENT) );
  _hal->powerVoltageRegulator( PWR_OUT_1, true );

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator( PWR_OUT_3,_config.Get("vdda",CLICpix2_VDDA),
			     _config.Get("vdda_current", CLICpix2_VDDA_CURRENT) );
  _hal->powerVoltageRegulator( PWR_OUT_3, true );

  //FIXME:  _config.Get("cml_iref_pol",CLICpix2_CML_IREF_POL) ) doesn't compile
  LOG(logDEBUG) << " CML_IREF";
  _hal->setCurrentSource( CUR_1, _config.Get("cml_iref",CLICpix2_CML_IREF),
			  CLICpix2_CML_IREF_POL);
  _hal->powerCurrentSource(CUR_1, true); 

  // LOG(logDEBUG) << " DAC_IREF";
  // _hal->setCurrentSource( CUR_2, _config.Get("dac_iref",CLICpix2_DAC_IREF),
  // 			  CLICpix2_DAC_IREF_POL);
  // _hal->powerCurrentSource(CUR_2, true); 

}

void clicpix2::powerOff() {
  LOG(logINFO) << DEVICE_NAME << ": Power off CLICpix2";

  LOG(logDEBUG) << "Power off CML_IREF";
  _hal->powerCurrentSource(CUR_1, false); 

  LOG(logDEBUG) << "Power off DAC_IREF";
  _hal->powerCurrentSource(CUR_2, false); 
  
  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator( PWR_OUT_3, false );
  
  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator( PWR_OUT_1, false );
  
  LOG(logDEBUG) << "Power off VDDCML";
  _hal->powerVoltageRegulator( PWR_OUT_5, false );

  LOG(logDEBUG) << "Power off CMLBUFFERS_VCO";
  _hal->powerVoltageRegulator( PWR_OUT_7, false );

  LOG(logDEBUG) << "Power off CMLBUFFERS_VDD";
  _hal->powerVoltageRegulator( PWR_OUT_4, false );

}

void clicpix2::programMatrix() {

  // Use a boolean vector to construct full matrix data array:
  std::vector<bool> matrix;

  // Loop over all (half-) rows, start with lowest:
  for(size_t row = 0; row < 256; row++) {
    // Store 14 bit per pixel:
    for(size_t bit = 0; bit < 14; bit++) {
      // Loop over all double columns
      for(size_t dcolumn = 0; dcolumn < 64; dcolumn++) {
	// Send one bit per double column to form one 64bit word
	// FIXME matrix.push_back(pixels.at(row/2).at(2*dcolumn+row%2).GetBit(bit));
      }
    }

    // After every superpixel (16 pixels), add one flip-flop per double column:
    if((row+1)%16 == 0) { matrix.insert(matrix.end(),64,0); }
  }

  // At the end of the column, add one flip-flop per double column:
  matrix.insert(matrix.end(),64,0);
  // At the very end, write one 64bit word with zeros to blank matrix after readout:
  matrix.insert(matrix.end(),64,0);

  std::vector<uint8_t> spi_data;
  
  // Read matrix in 8b chunks to send over SPI interface:
  uint8_t word = 0;
  for(size_t bit = 0; bit < matrix.size(); bit++) {
    // Obey big-endianness of SPI: flip 8bit word endianness:
    word += (matrix.at(bit)<<(7-bit%8));
    if((bit+1)%8 == 0 ) {
      spi_data.push_back(word);
      word = 0;
    }
  }

  // Finally, send the data over the SPI interface:
  //_hal->sendCommand(0x4,spi_data);
}

void clicpix2::configureClock() {
  LOG(logINFO) << DEVICE_NAME << ": Configure clockx";
  _hal->configureSI5345( (SI5345_REG_T const * const) si5345_revb_registers , SI5345_REVB_REG_CONFIG_NUM_REGS);
}

void clicpix2::powerStatusLog(){
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_3) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_3) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_3) << "W";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_1) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_1) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_1) << "W";


  LOG(logINFO) << "VDDACML:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_5) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_5) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_5) << "W";


  LOG(logINFO) << "CMLBUFFERS_VCO:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_7) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_7) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_7) << "W";
  

  LOG(logINFO) << "CMLBUFFERS_VDD:";
  LOG(logINFO) << "\tBus voltage: " << _hal->measureVoltage(PWR_OUT_4) << "V";
  LOG(logINFO) << "\tBus current: " << _hal->measureCurrent(PWR_OUT_4) << "A";
  LOG(logINFO) << "\tBus power  : " << _hal->measurePower(PWR_OUT_4) << "W";
}

void clicpix2::exploreInterface(){
  LOG(logINFO) << DEVICE_NAME << " - Exploring interface capabilities...";

  std::pair<uint8_t, uint8_t> pairdata = std::make_pair(0x12,0x34);
  LOG(logDEBUG) << "Write: Pair of register and data word";
  LOG(logDEBUG) << "  sending:   " << to_hex_string(pairdata.first) << " -> " << to_hex_string(pairdata.second);
  std::pair<uint8_t, uint8_t> pairval = _hal->getInterface<iface_spi>().send(pairdata);
  LOG(logDEBUG) << "  receiving: " << to_hex_string(pairval.first) << " -> " << to_hex_string(pairval.second);

  std::vector<uint8_t> vecdata {0x34, 0x56, 0x78};
  LOG(logDEBUG) << "Write: Register plus vector of data words";
  LOG(logDEBUG) << "  sending:   " << to_hex_string(0x12) << " -> " << listVector(vecdata,",",true);
  LOG(logDEBUG) << "  receiving: " << listVector(_hal->getInterface<iface_spi>().send(0x12, vecdata),",", true);

  std::vector<std::pair<uint8_t, uint8_t>> pairvec {std::make_pair(0x12,0x34), std::make_pair(0x56,0x78), std::make_pair(0x90,0xab)};
  LOG(logDEBUG) << "Write: Vector of register/data word pairs";
  LOG(logDEBUG) << "  sending:   " << listVector(pairvec,",",true);
  LOG(logDEBUG) << "  receiving: " << listVector(_hal->getInterface<iface_spi>().send(pairvec),",", true);
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  clicpix2* mDevice = new clicpix2(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
