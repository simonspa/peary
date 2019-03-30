#include <cstdint>
#include <fstream>
#include <iostream>
#include <unistd.h>

struct pixelhit {

  uint32_t col = 0;
  uint32_t row = 0;
  uint32_t ts1 = 0;
  uint32_t ts2 = 0;
  uint64_t fpga_ts = 0;
  uint32_t tot = 0;
  uint32_t SyncedTS = 0;
  uint32_t triggercnt;
  uint32_t ATPbinaryCnt;
  uint32_t ATPGreyCnt;
};

pixelhit decodeHit(uint32_t hit) {
  pixelhit tmp;

  tmp.col = (hit >> 25) & 0b11111;

  tmp.row = (hit >> 16) & 0x1FF;
  tmp.ts1 = (hit >> 6) & 0x3FF;
  tmp.ts2 = hit & 0x3F;

  if(((tmp.ts1 * 2) & 0x3F) > tmp.ts2) {
    tmp.tot = 64 - ((tmp.ts1 * 2) & 0x3F) + tmp.ts2;
  } else {
    tmp.tot = +tmp.ts2 - ((tmp.ts1 * 2) & 0x3F);
  }

  return tmp;
}

int main(int argc, char** argv) {

  uint32_t din;

  std::ifstream bindata;
  // std::ofstream txtdata;
  std::streampos ifilepos, ifileend;

  uint64_t fpga_tsx = 0;
  uint64_t fpga_ts1 = 0;
  uint64_t fpga_ts2 = 0;
  uint64_t fpga_ts3 = 0;
  uint64_t fpga_ts = 0;
  uint32_t timestamp = 0;
  uint32_t TrCNT = 0;

  bool param_tail = false;
  bool param_follow = false;

  argc--;
  if(!argc) {
    std::cout << "USAGE: " << argv[0] << " [-t] [-f] <binary_data_file_to_be_parsed>" << std::endl;
    return -1;
  }

  bindata.open(argv[argc], std::ios::in | std::ios::binary);
  // txtdata.open("data.txt", std::ios::out);

  if(!bindata.is_open()) {
    std::cout << "INFO: Waiting for input file \"" << argv[argc] << "\" to be created..." << std::endl;
    do {
      usleep(250000);
      bindata.open(argv[1], std::ios::in | std::ios::binary);
    } while(!bindata.is_open());
  }

  std::cout << "INFO: File \"" << argv[argc] << "\" opened." << std::endl;

  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 't':
        param_tail = true;
        break;
      case 'f':
        param_follow = true;
        break;
      default:
        std::cout << "Unknown parameter -" << argv[i][1] << std::endl;
        return -1;
      }
    } else {
      std::cout << "Invalid argument " << argv[i] << std::endl;
      return -1;
    }
  }

  if(param_tail) {
    bindata.seekg(0, bindata.end);
    ifileend = bindata.tellg();
    if((int)ifileend & 0b11) {
      std::cout << "ERROR: Input file is not aligned!" << std::endl;
      return -2;
    }
    ifileend -= 4;
    if((int)ifileend >= 0) {
      bindata.seekg(ifileend);
    }
  }

  std::cout << "INFO: Starting read at position " << (int)bindata.tellg() << ", entry no. " << ((int)bindata.tellg() >> 2)
            << "." << std::endl;

  while(true) {

    while(true) {
      bindata.read((char*)&din, 4);

      if(bindata.eof()) {
        if(param_follow) {
          usleep(250000);
          bindata.clear();
          ifilepos = bindata.tellg();
          bindata.seekg(0, bindata.end);
          ifileend = bindata.tellg();
          if(ifileend < ifilepos) {
            if((int)ifileend & 0b11) {
              std::cout << "ERROR: Input file is not aligned!" << std::endl;
              return -2;
            }
            ifileend -= 4;
            if((int)ifileend >= 0) {
              bindata.seekg(ifileend);
            }
            std::cout << "INFO: File truncated. New file read position: " << ifileend
                      << " Previous read position: " << ifilepos << std::endl;
          } else {
            bindata.seekg(ifilepos);
          }
        } else {
          return 0;
        }
      } else {
        break;
      }
    }

    if((din >> 31) == 1) {

      pixelhit hit = decodeHit(din);
      // LOG(INFO) << hit.col <<" " << hit.row << " " << hit.ts1 << ' ' << hit.ts2 << std::endl;
      std::cout << "HIT\t" << hit.col << "\t" << hit.row << "\t" << hit.ts1 << "\t" << hit.ts2 << "\t" << hit.tot << "\t"
                << fpga_ts << "\t" << TrCNT << "\t" << timestamp << std::endl;
      // disk << std::bitset<32>(d1) << std::endl;
    }

    else {

      uint32_t data_type = (din >> 24) & 0xFF;

      // Parse the different data types (BUFFEROVERFLOW,TRIGGER,BUSY_ASSERTED)
      switch(data_type) {

      case 0b01000000: // BinCnt from ATLASPix, not read for now
        timestamp = (din >> 8) & 0xFFFF;
        // std::cout << "TS_READOUT " << timestamp << std::endl;

        break;
      case 0b00000001: // Buffer overflow, data after this are lost
        std::cout << "BUFFER_OVERFLOW" << std::endl;
        break;
      case 0b00010000: // Trigger cnt 24bits
        TrCNT = din & 0xFFFFFF;
        break;
      case 0b00110000: // Trigger cnt 8b + fpga_ts 16 bits
        TrCNT = TrCNT + ((din << 8) & 0xFF000000);
        // std::cout << "TRIGGER " << TrCNT << std::endl;
        fpga_ts1 = (((uint64_t)din << 48) & 0xFFFF000000000000);
        // std::cout << "TS_FPGA_1 " << fpga_ts1 << std::endl;
        break;
      case 0b00100000: // continuation of fpga_ts (24 bits)
        fpga_tsx = (((uint64_t)din << 24) & 0x0000FFFFFF000000);
        // std::cout << "TS_FPGA_2 " << fpga_tsx << std::endl;
        fpga_ts2 = fpga_tsx;
        break;
      case 0b01100000: // End of fpga_ts (24 bits)
        fpga_tsx = ((din)&0xFFFFFF);
        // std::cout << "TS_FPGA_3 " << fpga_tsx << std::endl;
        fpga_ts3 = fpga_tsx;
        fpga_ts = fpga_ts1 | fpga_ts2 | fpga_ts3;
        // std::cout << "TS_FPGA " << fpga_ts << std::endl;
        break;
      case 0b00000010: // BUSY asserted with 24bit LSB of Trigger FPGA TS
        std::cout << "BUSY_ASSERTED " << (din & 0xFFFFFF) << std::endl;
        break;
      case 0b01110000: // T0 registered
        fpga_ts1 = 0;
        fpga_ts2 = 0;
        fpga_ts3 = ((din)&0xFFFFFF);
        fpga_ts = fpga_ts3;
        std::cout << "T0 " << (din & 0xFFFFFF) << std::endl;
        break;
      case 0b00001100: // SERDES lock lost
        std::cout << "SERDES_LOCK_LOST" << std::endl;
        break;
      case 0b00001000: // SERDES lock established
        std::cout << "SERDES_LOCK_ESTABLISHED" << std::endl;
        break;
      case 0b00000100: // Unexpected/weird data came
        std::cout << "WEIRD_DATA" << std::endl;
        break;
      default: // weird stuff, should not happend
        std::cout << "I AM IMPOSSIBLE!!!!!!!!!!!!!!!!!!" << std::endl;
        break;
      }
    }
  }

  // txtdata.close();
  bindata.close();
}
