// Implementation of the CLICpix2 frame decoer

#include "clicpix2_frameDecoder.hpp"
#include <cmath>
#include <iterator>
#include "exceptions.hpp"

using namespace caribou;

const clicpix2_frameDecoder::WORD_TYPE clicpix2_frameDecoder::DELIMITER(1, 0xf7);

void clicpix2_frameDecoder::decode(const std::vector<uint32_t> frame) {
  std::vector<WORD_TYPE> dataVector = repackageFrame(frame);
  auto data = dataVector.cbegin();
  auto dataEnd = dataVector.cend();

  do {
    decodeHeader(*data++); // header
    extractColumns(data, dataEnd);
  } while(std::distance(data, dataEnd) && ~(std::distance(data, dataEnd) == 1 && *data == DELIMITER));

  // FIXEME: DECODE_LFSR_COUNTERS
  // For this one needs to know matrix configuration
}

std::vector<clicpix2_frameDecoder::WORD_TYPE> clicpix2_frameDecoder::repackageFrame(const std::vector<uint32_t> frame) {
  std::vector<WORD_TYPE> data;
  for(auto const& it : frame) {
    data.emplace_back((it >> 17) & 0x1, (it >> 8) & 0xFF); // MSByte
    data.emplace_back((it >> 16) & 0x1, it & 0xFF);        // LSByte
  }
  return data;
}

void clicpix2_frameDecoder::decodeHeader(const clicpix2_frameDecoder::WORD_TYPE word) {
  if(word.is_control != 0)
    throw DataException("Packet header should be a regular data word");

  rcr = (word.word >> 6) & 0x3;

  if(rcr == 0)
    throw DataException("Unsupported RCR in packet header");

  firstColumn = word.word & 0x1F;
}

void clicpix2_frameDecoder::extractColumns(std::vector<clicpix2_frameDecoder::WORD_TYPE>::const_iterator& data,
                                           std::vector<clicpix2_frameDecoder::WORD_TYPE>::const_iterator dataEnd) {
  std::array<std::array<pixelReadout, 8>, CLICPIX2_ROW * 2>
    pixels_dc; // stores results of the processed doube columns (up to 8 if compression is enabled)
  std::array<int, 8> row_index = {
    0, 0, 0, 0, 0, 0, 0, 0}; // 8 independent (in case of compression) counters navigating through the <pixels_dc>
  std::array<int, 8> row_slice = {
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
    CLICPIX2_PIXEL_SIZE - 1,
  }; // 8 independent
//(in case of compression) counters navigating through the <pixels_dc>
#define DC_COUNTER_INIT (2 * (CLICPIX2_ROW * CLICPIX2_PIXEL_SIZE + CLICPIX2_ROW / CLICPIX2_SUPERPIXEL_SIZE) + 1)
  std::array<int, 8> dc_counter = {DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT,
                                   DC_COUNTER_INIT}; // 8 independent (in case of compression)
  // counters indicating number of bits processed for the given double-column
  // value 3601 indicates beginning of the double-column
  std::array<int, 8> sp_counter = {
    0, 0, 0, 0, 0, 0, 0, 0}; // number of pixels in the processed super-pixel for the given column

  do {
    WORD_TYPE word = *data++;
    if(word == DELIMITER) // end of double column
      break;
    if(word.is_control)
      throw DataException("Found control word different than delimeter");

    unraveDC(pixels_dc, dc_counter, sp_counter, row_index, row_slice, word.word);
  } while(std::distance(data, dataEnd));

  for(int i = 0; i < pow(2, rcr); i++)
    if(dc_counter[i] != 3600)
      throw DataException("Partial double column");

  // remove snake pattern
  for(unsigned int r = 0; r < 256; ++r)
    for(unsigned int c = 0; c < pow(2, rcr); c++) {
      switch(r % 4) {
      case 0:
        matrix[r / 2][c * 2 * 64 / pow(2, rcr) + firstColumn * 2] = pixels_dc[r][c];
        break; // left column
      case 1:
        matrix[r / 2][c * 2 * 64 / pow(2, rcr) + firstColumn * 2 + 1] = pixels_dc[r][c];
        break; // right column
      case 2:
        matrix[r / 2][c * 2 * 64 / pow(2, rcr) + firstColumn * 2 + 1] = pixels_dc[r][c];
        break; // right column
      case 3:
        matrix[r / 2][c * 2 * 64 / pow(2, rcr) + firstColumn * 2] = pixels_dc[r][c];
        break; // left column
      }
    }
}

void clicpix2_frameDecoder::unraveDC(std::array<std::array<pixelReadout, 8>, CLICPIX2_ROW * 2>& pixels_dc,
                                     std::array<int, 8>& dc_counter,
                                     std::array<int, 8>& sp_counter,
                                     std::array<int, 8>& row_index,
                                     std::array<int, 8>& row_slice,
                                     const uint8_t data) {

  // unravel the double-columns
  switch(rcr) {
  case 1:
    for(int i = 0; i < 8; i += 2) {
      processDCbit(pixels_dc, dc_counter[0], sp_counter[0], row_index[0], 0, row_slice[0], (data >> (i)) & 0x1);
      processDCbit(pixels_dc, dc_counter[1], sp_counter[1], row_index[1], 1, row_slice[1], (data >> (i + 1)) & 0x1);
    }
    break;
  case 2:
    for(int i = 0; i < 8; i += 4) {
      processDCbit(pixels_dc, dc_counter[0], sp_counter[0], row_index[0], 0, row_slice[0], (data >> (i)) & 0x1);
      processDCbit(pixels_dc, dc_counter[1], sp_counter[1], row_index[1], 1, row_slice[1], (data >> (i + 1)) & 0x1);
      processDCbit(pixels_dc, dc_counter[2], sp_counter[2], row_index[2], 2, row_slice[2], (data >> (i + 2)) & 0x1);
      processDCbit(pixels_dc, dc_counter[3], sp_counter[3], row_index[3], 3, row_slice[3], (data >> (i + 3)) & 0x1);
    }
    break;
  case 3:
    for(int i = 0; i < 8; i += 8) {
      processDCbit(pixels_dc, dc_counter[0], sp_counter[0], row_index[0], 0, row_slice[0], (data >> (i)) & 0x1);
      processDCbit(pixels_dc, dc_counter[1], sp_counter[1], row_index[1], 1, row_slice[1], (data >> (i + 1)) & 0x1);
      processDCbit(pixels_dc, dc_counter[2], sp_counter[2], row_index[2], 2, row_slice[2], (data >> (i + 2)) & 0x1);
      processDCbit(pixels_dc, dc_counter[3], sp_counter[3], row_index[3], 3, row_slice[3], (data >> (i + 3)) & 0x1);
      processDCbit(pixels_dc, dc_counter[4], sp_counter[4], row_index[4], 4, row_slice[4], (data >> (i + 4)) & 0x1);
      processDCbit(pixels_dc, dc_counter[5], sp_counter[5], row_index[5], 5, row_slice[5], (data >> (i + 5)) & 0x1);
      processDCbit(pixels_dc, dc_counter[6], sp_counter[6], row_index[6], 6, row_slice[6], (data >> (i + 6)) & 0x1);
      processDCbit(pixels_dc, dc_counter[7], sp_counter[7], row_index[7], 7, row_slice[7], (data >> (i + 7)) & 0x1);
    }
    break;
  }
}

void clicpix2_frameDecoder::processDCbit(std::array<std::array<pixelReadout, 8>, CLICPIX2_ROW * 2>& pixels_dc,
                                         int& dc_counter,
                                         int& sp_counter,
                                         int& row_index,
                                         const int col_index,
                                         int& row_slice,
                                         const bool data) {

  // middle of the double-column
  if(dc_counter < static_cast<int>(2 * (CLICPIX2_ROW * CLICPIX2_PIXEL_SIZE + CLICPIX2_ROW / CLICPIX2_SUPERPIXEL_SIZE))) {
    // super-pixel bit
    if(sp_counter == 0) {
      if(data ||                               // not empty super-pixel
         !DCandSuperPixelCompressionEnabled) { // or sp compression disabled
        dc_counter++;
        sp_counter++;
      } else { // empty super-pixel

        for(auto i = 0; i < static_cast<int>(CLICPIX2_SUPERPIXEL_SIZE); ++i)
          pixels_dc[row_index++][col_index].setLatches(0x00);

        dc_counter += CLICPIX2_SUPERPIXEL_SIZE * CLICPIX2_PIXEL_SIZE + 1;
      }
    }

    // pixel bit
    else if(row_slice == CLICPIX2_PIXEL_SIZE - 1) {

      if(data ||                     // not empty pixel
         !pixelCompressionEnabled) { // or pixel compression disabled
        pixels_dc[row_index][col_index].setLatches(data, row_slice--);
        dc_counter++;
        sp_counter++;
      } else { // empty pixel
        pixels_dc[row_index++][col_index].setLatches(0x00);
        dc_counter += CLICPIX2_PIXEL_SIZE;
        sp_counter += CLICPIX2_PIXEL_SIZE;
      }
    }

    // pixel payload
    else {
      pixels_dc[row_index][col_index].setLatches(data, row_slice--);
      dc_counter++;
      sp_counter++;

      if(row_slice < 0) {
        row_index++;
        row_slice = CLICPIX2_PIXEL_SIZE - 1;
      }
    }

    if(sp_counter > static_cast<int>(CLICPIX2_SUPERPIXEL_SIZE * CLICPIX2_PIXEL_SIZE)) // reset sp_counter
      sp_counter = 0;

  }
  // beginning of the double-column (dc_counter == 3601)
  else {
    if(dc_counter ==
       static_cast<int>(2 * (CLICPIX2_ROW * CLICPIX2_PIXEL_SIZE + CLICPIX2_ROW / CLICPIX2_SUPERPIXEL_SIZE) + 1)) {
      if(data ||                             // not empty double-column
         !DCandSuperPixelCompressionEnabled) // or collumn compression disabled
        dc_counter = 0;
      else { // empty double-column
        for(auto i = 0; i < static_cast<int>(2 * CLICPIX2_ROW); ++i)
          pixels_dc[row_index++][col_index].setLatches(0x00);

        dc_counter = 2 * (CLICPIX2_ROW * CLICPIX2_PIXEL_SIZE + CLICPIX2_ROW / CLICPIX2_SUPERPIXEL_SIZE); // 3600
      }
    }
  }
}

namespace caribou {
  std::ostream& operator<<(std::ostream& out, const clicpix2_frameDecoder& decoder) {
    for(auto r = 0; r < static_cast<int>(clicpix2_frameDecoder::CLICPIX2_ROW); ++r)
      for(auto c = 0; c < static_cast<int>(clicpix2_frameDecoder::CLICPIX2_COL); ++c) {
        out << "[" << r << "][" << c << "] " << decoder.matrix[r][c] << ", ";
        if(c % 64 == 63)
          out << "\n";
      }
    return out;
  }
}
