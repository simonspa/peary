#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "clicpix2_frameDecoder.hpp"
#include "clicpix2_utilities.hpp"
#include "configuration.hpp"
#include "devicemgr.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;
using namespace clicpix2_utils;

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
  try {
    conf = clicpix2_utils::readMatrix(matrixfile);
    LOG(logINFO) << "Matrix configuration size: " << conf.size();
  } catch(ConfigInvalid&) {
    return 1;
  }

  LOG(logQUIET) << "Reading Clicpix2 rawdata from: " << datafile;
  std::ifstream f;
  std::ofstream outfile;
  f.open(datafile);
  outfile.open(datafile + ".txt");
  std::string line;

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
          // std::cout << h << std::endl;
          outfile << h << "\n";
        }
        std::cout << header[0] << std::endl;

        decoder.decode(rawData, conf);
        pearydata data = decoder.getZerosuppressedFrame();
        for(const auto& px : data) {
          outfile << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
          // std::cout << px.first.first << "," << px.first.second << "," << (*px.second) << std::endl;
        }
        std::cout << data.size() << " pixel responses" << std::endl;
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
