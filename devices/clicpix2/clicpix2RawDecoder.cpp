#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "clicpix2_frameDecoder.hpp"
#include "clicpix2_utilities.hpp"

#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;
using namespace clicpix2_utils;

int main(int argc, char* argv[]) {

  std::string datafile, matrixfile;

  Log::ReportingLevel() = TLogLevel::logINFO;

  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-d datafile    data file to be decoded" << std::endl;
      std::cout << "-m matrixfile  matrix configuration to read pixel states from" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
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
    // Make sure we initializefd all pixels:
    for(size_t column = 0; column < 128; column++) {
      for(size_t row = 0; row < 128; row++) {
        pixelConfig px = conf[std::make_pair(row, column)];
      }
    }
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

  // Compression flags
  bool comp = true;
  bool sp_comp = true;

  // Parse the header:
  while(getline(f, line)) {

    if(!line.length()) {
      continue;
    }
    if('#' != line.at(0)) {
      break;
    }

    LOG(logDEBUG) << "Detected file header: " << line;

    // Search for compression settings:
    std::string::size_type n = line.find(" sp_comp:");
    if(n != std::string::npos) {
      LOG(logDEBUG) << "Value read for sp_comp: " << line.substr(n + 9, 1);
      sp_comp = static_cast<bool>(std::stoi(line.substr(n + 9, 1)));
      LOG(logINFO) << "Superpixel Compression: " << (sp_comp ? "ON" : "OFF");
    }
    n = line.find(" comp:");
    if(n != std::string::npos) {
      LOG(logDEBUG) << "Value read for comp: " << line.substr(n + 6, 1);
      comp = static_cast<bool>(std::stoi(line.substr(n + 6, 1)));
      LOG(logINFO) << "     Pixel Compression: " << (comp ? "ON" : "OFF");
    }
  }

  clicpix2_frameDecoder decoder(comp, sp_comp);
  LOG(logDEBUG) << "finished reading file header, now data should follow...";

  // Parse the main body
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
