/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

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
  _hal->setVoltageRegulator( PWR_OUT4,_config.Get("cmlbuffers_vdd", CLICpix2_CMLBUFFERS_VDD) );
  _hal->powerVoltageRegulator( PWR_OUT4, true );

  LOG(logDEBUG) << " CMLBUFFERS_VCO";
  _hal->setVoltageRegulator( PWR_OUT7,_config.Get("cmlbuffers_vco", CLICpix2_CMLBUFFERS_VCO) );
  _hal->powerVoltageRegulator( PWR_OUT7, true );
  
  LOG(logDEBUG) << " VDDCML";
  _hal->setVoltageRegulator( PWR_OUT5,_config.Get("vdddcml",CLICpix2_VDDCML) );
  _hal->powerVoltageRegulator( PWR_OUT5, true );
  
  LOG(logDEBUG) << " VDDD";
  _hal->setVoltageRegulator( PWR_OUT1,_config.Get("vddd",CLICpix2_VDDD) );
  _hal->powerVoltageRegulator( PWR_OUT1, true );

  LOG(logDEBUG) << " VDDA";
  _hal->setVoltageRegulator( PWR_OUT3,_config.Get("vdda",CLICpix2_VDDA) );
  _hal->powerVoltageRegulator( PWR_OUT3, true );

}

void clicpix2::powerOff() {
  LOG(logINFO) << DEVICE_NAME << ": Power off CLICpix2";

  LOG(logDEBUG) << "Power off VDDA";
  _hal->powerVoltageRegulator( PWR_OUT3, false );
  
  LOG(logDEBUG) << "Power off VDDD";
  _hal->powerVoltageRegulator( PWR_OUT1, false );
  
  LOG(logDEBUG) << "Power off VDDCML";
  _hal->powerVoltageRegulator( PWR_OUT5, false );

  LOG(logDEBUG) << "Power off CMLBUFFERS_VCO";
  _hal->powerVoltageRegulator( PWR_OUT7, false );

  LOG(logDEBUG) << "Power off CMLBUFFERS_VDD";
  _hal->powerVoltageRegulator( PWR_OUT4, false );

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

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  clicpix2* mDevice = new clicpix2(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
