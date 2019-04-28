#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "clicpix2_utilities.hpp"
#include "framedecoder/clicpix2_frameDecoder.hpp"

#include "utils/exceptions.hpp"
#include "utils/log.hpp"

using namespace caribou;
using namespace clicpix2_utils;

int main(int argc, char* argv[]) {

  std::string datafile, matrixfile;

  Log::setReportingLevel(LogLevel::INFO);

  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-d datafile    data file to be decoded" << std::endl;
      std::cout << "-m matrixfile  matrix configuration to read pixel states from" << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      try {
        LogLevel log_level = Log::getLevelFromString(std::string(argv[++i]));
        Log::setReportingLevel(log_level);
      } catch(std::invalid_argument& e) {
        LOG(ERROR) << "Invalid verbosity level \"" << std::string(argv[i]) << "\", ignoring overwrite";
      }
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

  LOG(STATUS) << "Reading Clicpix2 rawdata from: " << datafile;
  std::ifstream f;
  std::ofstream outfile;
  f.open(datafile);
  size_t lastindex = datafile.find_last_of(".");
  outfile.open(datafile.substr(0, lastindex) + ".csv");
  std::string line;

  // Compression flags
  bool comp = true;
  bool sp_comp = true;

  std::streampos oldpos;

  // Parse the header:
  while(getline(f, line)) {

    if(!line.length()) {
      continue;
    }
    if('#' != line.at(0)) {
      break;
    }

    // Replicate header to new file:
    LOG(DEBUG) << "Detected file header: " << line;
    outfile << line << "\n";
    oldpos = f.tellg();

    // Search for compression settings:
    std::string::size_type n = line.find(" sp_comp:");
    if(n != std::string::npos) {
      LOG(DEBUG) << "Value read for sp_comp: " << line.substr(n + 9, 1);
      sp_comp = static_cast<bool>(std::stoi(line.substr(n + 9, 1)));
      LOG(INFO) << "Superpixel Compression: " << (sp_comp ? "ON" : "OFF");
    }
    n = line.find(" comp:");
    if(n != std::string::npos) {
      LOG(DEBUG) << "Value read for comp: " << line.substr(n + 6, 1);
      comp = static_cast<bool>(std::stoi(line.substr(n + 6, 1)));
      LOG(INFO) << "     Pixel Compression: " << (comp ? "ON" : "OFF");
    }
  }

  clicpix2_frameDecoder decoder(comp, sp_comp, conf);
  LOG(INFO) << "Finished reading file header, now decoding data...";

  std::vector<std::string> header;
  std::vector<uint32_t> rawData;
  unsigned int frames = 0;

  // Parse the main body
  f.seekg(oldpos);
  while(getline(f, line)) {
    // Ignore empty lines and comments:
    if(!line.length() || '#' == line.at(0)) {
      continue;
    }

    // New frame, write old one
    if(line.find("====") != std::string::npos) {
      LOG(DEBUG) << "Found new frame header: " << line;

      // decode and write old frame
      if(!rawData.empty() && !header.empty()) {
        LOG(DEBUG) << "Raw data of previous frame available, length: " << rawData.size();
        LOG(DEBUG) << "Writing header:";
        for(const auto& h : header) {
          LOG(DEBUG) << h;
          outfile << h << "\n";
        }

        LOG(DEBUG) << "Writing decoded data:";
        try {
          decoder.decode(rawData);
          pearydata data = decoder.getZerosuppressedFrame();
          for(const auto& px : data) {
            outfile << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
            LOG(DEBUG) << px.first.first << "," << px.first.second << "," << (*px.second);
          }
          LOG(INFO) << header.front() << ": " << data.size() << " pixel responses";
          frames++;
        } catch(caribou::DataException& e) {
          LOG(ERROR) << "Caugth DataException: " << e.what() << ", clearing event data.";
        }

        header.clear();
        rawData.clear();
      } else {
        LOG(DEBUG) << "No frame data available, collecting...";
      }

      // Add newly found header:
      header.push_back(line);
    }
    // timestamps
    else if(line.find(":") != std::string::npos) {
      header.push_back(line);
    }
    // Pixel hits
    else {
      rawData.push_back(atoi(line.c_str()));
    }
  }

  f.close();
  outfile.close();
  LOG(STATUS) << "...all written: " << frames << " frames.";

  return 0;
}
