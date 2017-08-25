#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "clicpix2_frameDecoder.hpp"
#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

std::map<std::pair<uint8_t, uint8_t>, pixelConfig> readMatrix(std::string filename) {

  std::map<std::pair<uint8_t, uint8_t>, pixelConfig> pixelsConfig;
  size_t masked = 0;
  std::cout << "Reading pixel matrix file." << std::endl;
  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    std::cout << "Could not open matrix file \"" << filename << "\"" << std::endl;
    return pixelsConfig;
  }

  std::string line = "";
  while(std::getline(pxfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pxline(line);
    int column, row, threshold, mask, cntmode, tpenable, longcnt;
    if(pxline >> row >> column >> mask >> threshold >> cntmode >> tpenable >> longcnt) {
      pixelConfig px(mask, threshold, cntmode, tpenable, longcnt);
      pixelsConfig[std::make_pair(row, column)] = px;
      if(mask)
        masked++;
    }
  }
  std::cout << "Now " << pixelsConfig.size() << " pixel configurations cached, " << masked << " of which are masked"
            << std::endl;
  return pixelsConfig;
}

int main(int argc, char* argv[]) {

  std::string datafile, matrixfile;

  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-d datafile    data file to be decoded" << std::endl;
      std::cout << "-m matrixfile  matrix configuration to read pixel states from" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-d")) {
      datafile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-m")) {
      matrixfile = std::string(argv[++i]);
      continue;
    } else {
      std::cout << "Unrecognized argument: " << argv[i] << std::endl;
    }
  }

  std::map<std::pair<uint8_t, uint8_t>, pixelConfig> conf;
  conf = readMatrix(matrixfile);
  std::cout << "Conf size: " << conf.size() << std::endl;
  std::cout << "Reading Clicpix2 rawdata from: " << datafile << std::endl;
  std::ifstream f;
  std::ofstream outfile;
  f.open(datafile);
  outfile.open(datafile + ".txt");
  std::string line;
  unsigned int framecounter = 0;
  std::vector<std::string> header;
  std::vector<uint32_t> rawData;

  uint32_t comp = 1;
  uint32_t sp_comp = 1;

  clicpix2_frameDecoder decoder((bool)comp, (bool)sp_comp);

  while(getline(f, line)) {
    // New frame, write old one
    if(line.find("====") != std::string::npos) {

      // decode and write old frame
      if(rawData.size() > 0) {
        for(const auto& h : header) {
          std::cout << h << std::endl;
        }

        decoder.decode(rawData, conf);
        pearydata data = decoder.getZerosuppressedFrame();
        for(const auto& px : data) {
          // outfile << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
          std::cout << px.first.first << "," << px.first.second << "," << (*px.second) << std::endl;
        }
        header.clear();
        rawData.clear();
      }
      header.push_back(line);

      continue;
    }
    // timestamps
    if(line.find(":") != std::string::npos) {
      header.push_back(line);
      // std::cout << line <<std::endl;
      continue;
    }
    // Pixel hits
    else {
      rawData.push_back(atoi(line.c_str()));
      // std::cout<<rawData[rawData.size()-1]<<std::endl;
      continue;
    }
  }

  f.close();
  outfile.close();

  return 0;
}
