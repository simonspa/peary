/**
 * Caribou Device implementation for CLICpix2
 */

#include "clicpix2.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

void clicpix2::init() {
  LOG(logDEBUG) << "Initializing " << DEVICE_NAME;
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

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  clicpix2* mDevice = new clicpix2(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
