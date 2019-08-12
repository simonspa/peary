#include "CLICTDFrameDecoder.hpp"

using namespace caribou;

uint32_t CLICTDFrameDecoder::getNextPixel(const std::vector<uint32_t>& rawFrame, unsigned& word, unsigned& bit) {

  // out of range
  if(word >= rawFrame.size()) {
    return 0;
  }
  // if the next pixel is compressed / zero suppressed, it takes only one bit
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
  // we need to read full 22 bits
  else {
    uint32_t pixeldata;
    // full pixel information is contained in the element
    // and we do not need to move to the next vector element
    // (there is still something left belonging to next pixel)
    if(bit > (CLICTD_PIXEL_BITS - 1)) {
      pixeldata = rawFrame.at(word) >> (bit - (CLICTD_PIXEL_BITS - 1));
      bit -= CLICTD_PIXEL_BITS;
    } else {
      // we need to bring some bits from the next element of rawFrame vector
      if(bit < (CLICTD_PIXEL_BITS - 1)) {
        unsigned missing = (CLICTD_PIXEL_BITS - 1) - bit;
        pixeldata = rawFrame.at(word) << missing;
        pixeldata &= (0xFFFFFFFF << missing);
        if(++word >= rawFrame.size()) {
          LOG(ERROR) << "Reached the end of the frame but there still should be pixels. Possibly some alignment error or "
                        "incomplete frame?";
          return 0;
        }
        pixeldata |= (rawFrame.at(word) >> (32 - missing));
        bit = 31 - missing;
      }
      // bit pointer is 21
      // we do not need to shift anything but we need to move to the next vector element
      else {
        pixeldata = rawFrame.at(word);
        word++;
        bit = 31;
      }
    }
    // mask upper bits
    pixeldata &= (0xFFFFFFFFu >> (32 - CLICTD_PIXEL_BITS));
    return pixeldata;
  }
}

pearydata CLICTDFrameDecoder::decodeFrame(const std::vector<uint32_t>& rawFrame) {
  unsigned wrd = 0;
  unsigned bit = 31;
  pearydata data;

  if(getNextPixel(rawFrame, wrd, bit) != CLICTD_FRAME_START) {
    LOG(ERROR) << "The first word does not match the frame start pattern.";
    return data;
  }

  for(uint8_t col = 0; col < CLICTD_COLUMNS; col++) {
    // start of column
    uint32_t bits_of_data = getNextPixel(rawFrame, wrd, bit);
    if((bits_of_data & ~CLICTD_COLUMN_ID_MASK) != CLICTD_COLUMN_ID) {
      LOG(ERROR) << "Column " << col << " header does not match the pattern.";
      return data;
    }
    if(((bits_of_data & CLICTD_COLUMN_ID_MASK) >> CLICTD_COLUMN_ID_MASK_SHIFT) != col) {
      LOG(ERROR) << "Column " << col << " header does not match the expected column number.";
      return data;
    }
    // row data
    for(uint8_t row = 0; row < CLICTD_ROWS; row++) {
      // get data
      bits_of_data = getNextPixel(rawFrame, wrd, bit);
      data[std::make_pair(col, row)] = std::make_unique<CLICTDPixelReadout>(bits_of_data, longcnt);
    }
  }
  if(getNextPixel(rawFrame, wrd, bit) != CLICTD_FRAME_END) {
    LOG(ERROR) << "The last word does not match the frame end pattern.";
    return data;
  }
  return data;
}

std::vector<uint32_t> CLICTDFrameDecoder::splitFrame(const std::vector<uint32_t>& rawFrame) {
  unsigned wrd = 0;
  unsigned bit = 31;
  std::vector<uint32_t> data;

  data.push_back(getNextPixel(rawFrame, wrd, bit));

  for(uint8_t col = 0; col < CLICTD_COLUMNS; col++) {
    // start of column
    data.push_back(getNextPixel(rawFrame, wrd, bit));

    // row data
    for(uint8_t row = 0; row < CLICTD_ROWS; row++) {
      // get data
      data.push_back(getNextPixel(rawFrame, wrd, bit));
    }
  }
  data.push_back(getNextPixel(rawFrame, wrd, bit));

  return data;
}

/*
for(auto& px : data) {
  LOG(DEBUG) << px;
}
*/
