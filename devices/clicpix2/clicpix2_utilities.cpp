#include "clicpix2_utilities.hpp"
#include <fstream>
#include "log.hpp"

using namespace caribou;

std::map<std::pair<uint8_t, uint8_t>, pixelConfig> clicpix2_utils::readMatrix(std::string filename) {

  std::map<std::pair<uint8_t, uint8_t>, pixelConfig> pixelsConfig;
  size_t masked = 0;
  LOG(logDEBUG) << "Reading pixel matrix file.";
  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    LOG(logERROR) << "Could not open matrix file \"" << filename << "\"";
    throw ConfigInvalid("Could not open matrix file \"" + filename + "\"");
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
  LOG(logINFO) << pixelsConfig.size() << " pixel configurations cached, " << masked << " of which are masked";
  return pixelsConfig;
}
