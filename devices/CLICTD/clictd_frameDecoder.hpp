#ifndef CLICTD_FRAMEDECODER_HPP
#define CLICTD_FRAMEDECODER_HPP

#include <vector>

#define CLICTD_FRAME_START 0b1111111111111110101000
#define CLICTD_FRAME_END 0b1111111111111110010100
#define CLICTD_COLUMN_ID 0b1111111111111101000000
#define CLICTD_COLUMN_ID_MASK 0b0000000000000000111100
#define CLICTD_PIXEL_BITS 22
#define CLICTD_COLUMNS 16
#define CLICTD_ROWS 128

uint32_t getNextPixel(const std::vector<uint32_t>& rawFrame, unsigned* word, unsigned* bit);

pearydata decodeFrame(const std::vector<uint32_t>& rawFrame, const bool longcnt);

#endif
