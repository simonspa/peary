#include "clictd_frame.hpp"

uint32_t getNextPixel(const std::vector<uint32_t>& rawFrame, unsigned* word, unsigned* bit) {

  // out of range
  if(word >= rawFrame.size()) {
    return 0;
  }
  // if the next pixel is compressed / zero supressed, it takes only one bit
  if(!((rawFrame.at(word) >> bit) & 0b1)) {
    // move pointer to next pixel
    if(bit == 0) {
      bit = 31; // std::numeric_limits<uint32_t>::digits - 1;
      word++;
    } else {
      bit--;
    }
    // return empty pixel
    return 0;
  }
  // we need to read the full 22 bits
  else {
    uint32_t pixeldata;
    // full pixel information is contained in the element
    // and we do not need to move to the next vector element
    if(bit > (CLICTD_PIXEL_BITS - 1)) {
      pixeldata = rawFrame.at(word) >> (bit - (CLICTD_PIXEL_BITS - 1));
      bit = bit - CLICTD_PIXEL_BITS;
    } else {
      // we need to bring some bits from the next element
      if(bit < (CLICTD_PIXEL_BITS - 1)) {
        unsigned missing = (CLICTD_PIXEL_BITS - 1) - bit;
        pixeldata = rawFrame.at(word) << missing;
        pixeldata &= (0xFFFFFFFF << missing);
        if(++word >= rawFrame.size()) {
          LOG(ERROR) << "Reached the end of the frame but there still should be pixels. Possibly some alignment error?";
        }
        pixeldata |= (rawFrame.at(word) >> (32 - mising));
        bit = 31 - missing;
      }
      // we do not need to shift anything, but we need to move to next vector element
      else {
        pixeldata = rawFrame.at(word);
        word++;
        bit = 31;
      }
    }
    // mask upper bits
    pixeldata &= (0xFFFFFFFFu >> (32 - CLICTD_PIXEL_BITS);
    return pixeldata;
  }
}

pearydata decodeFrame(const std::vector<uint32_t>& rawFrame, const bool longcnt) {
  unsigned wrd = 0;
  unsigned bit = 31;
  pearydata data;

  if ((getNextPixel(rawData, &wrd, &bit) != CLICTD_FRAME_START) {
    LOG(ERROR) << "The first word does not match the frame start pattern.";
    return;
  }

  for (uint8_t col = 0; col < CLICTD_COLUMNS; col++) {
    // start of column
    uint32_t bits_of_data = getNextPixel(rawData, &wrd, &bit);
    if((bits_of_data & ~CLICTD_COLUMN_ID_MASK) != CLICTD_COLUMN_ID) {
      LOG(ERROR) << "Column " << col << " header does not match the pattern.";
      return;
    }
    if(((bits_of_data & CLICTD_COLUMN_ID_MASK) >> 2) != col) {
      LOG(ERROR) << "Column " << col << " header does not match the expected column number.";
      return;
    }
    // row data
    for(uint8_t row = 0; row < CLICTD_ROWS; row++) {
      // get data
      bits_of_data = getNextPixel(rawData, &wrd, &bit);
      data[std::make_pair(col, row)] = std::make_unique<CLICTDDevice::pixelReadout>();
      data[std::make_pair(col, row)]->setLatches(bits_of_data);
      data[std::make_pair(col, row)]->setLongFlag(longcnt);
    }
  }
  return pearydata;
}

/*
for(auto& px : data) {
  LOG(DEBUG) << px;
}
*/
