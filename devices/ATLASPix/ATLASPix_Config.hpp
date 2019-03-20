/*
 * ATLASPix_Config.hpp
 *
 *  Created on: 22 Sep 2017
 *      Author: peary
 */

#ifndef DEVICES_ATLASPIX_ATLASPIX_CONFIG_HPP_
#define DEVICES_ATLASPIX_ATLASPIX_CONFIG_HPP_

/****************************************************************************
 *                                                                          *
 * Class to generically generate bit vectors for configurating sensors      *
 *                                                                          *
 * Author: Rudolf Schimassek                                                *
 *                                                                          *
 * Version 2 (02.05.2017)                                                   *
 *              added support for arbitary permutations of the bits         *
 ****************************************************************************/

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class ATLASPix_Config {
public:
  ATLASPix_Config();

  enum ShiftDirection { MSBFirst = 0, LSBFirst = 1, GlobalInvertedMSBFirst = 2, GlobalInvertedLSBFirst = 3 };
  // when loading from a file, any other string than "LSBFirst" will be interpreted as "MSBFirst"

  /**
   * @brief adds a parameter to the configuration at the end
   * @param name              - the name of the parameter for reference/saving to a file
   * @param bits              - number of bits of the parameter/register
   * @param shiftdirection    - whether the MSB or the LSB is to be shifted in first
   * @param initial           - initial value
   * @return                  - true if the parameter was accepted, false if not
   */
  bool AddParameter(std::string name, unsigned int bits = 6, int shiftdirection = MSBFirst, unsigned int initial = 0);
  /**
   * @brief adds a parameter to the configuration at the end
   *
   * @param name     - the name of the parameter (will be saved in the maps)
   * @param bitorder - the comma separated ordering of the bits (limits: {1,...,30} (32bit signed int))
   * @param initial  - the initial value for the parameter
   * @return         - true if the parameter was accepted, false if not
   */
  bool AddParameter(std::string name, std::string bitorder = "5,4,3,2,1,0", unsigned int initial = 0);
  /**
   * @brief ClearParameters removes all parameters from the object
   */
  void ClearParameters();

  int GetParameterIndexToName(std::string name);
  std::string GetParameterNameToIndex(unsigned int index);

  /**
   * @brief provides the current value of the parameter
   * @param index     - the index of the parameter to return
   * @return          - the value of the parameter or -1 on an invalid index
   */
  int GetParameter(unsigned int index);
  /**
   * @brief provides the current value of the parameter
   * @param name      - reference name of the parameter
   * @return          - the value of the parameter or -1 on an invalid name
   */
  int GetParameter(std::string name);
  /**
   * @brief returns the number of bits of the parameter
   * @param index     - index of the parameter
   * @return          - the number of bits for this parameter or -1 for an invalid index
   */
  int GetParameterWidth(unsigned int index);
  /**
   * @brief returns the number of bits for the parameter requested
   * @param name      - reference name for the parameter to return
   * @return          - the number of bits for this parameter or -1 on an invalid name
   */
  int GetParameterWidth(std::string name);
  /**
   * @brief reuturns the order of the bits for this parameter
   * @param index     - index of the parameter to return
   * @return          - the comma separated order of the bits or an empty string on an invalid index
   */
  std::string GetParameterBitOrder(unsigned int index);
  /**
   * @brief returns the order of the bits for the parameter requested
   * @param name      - reference name of the index to return
   * @return          - the comma separated order of the bits or an empty string on invalid reference name
   */
  std::string GetParameterBitOrder(std::string name);

  /**
   * @brief returns the name of the parameter at `index`
   * @param index     - index of the parameter of interest
   * @return          - the parameter name or an empty string on invalid index
   */
  std::string GetParameterName(unsigned int index);

  /**
   * @brief changes the value of a parameter
   * @param index     - the index of the parameter to change
   * @param value     - the new value of the parameter
   * @return          - true if the index is valid and the new value is in the possible range, false otherwise
   */
  bool SetParameter(unsigned int index, unsigned int value);
  /**
   * @brief changes the value of a parameter
   * @param name      - the name of the parameter to change
   * @param value     - new value for the parameter
   * @return          - true on successful setting, false otherwise (wrong name, value out of bounds)
   */
  bool SetParameter(std::string name, unsigned int value);

  /**
   * @brief returns the number of entries in the object
   * @return
   */
  unsigned int GetEntries() { return parameters.size(); }

  /**
   * @brief generates a bitvector to send to a chip for configuration
   * @param shiftdirection    - for `LSBFirst`, the bit order of every parameter is reversed
   * @return                  - the resulting bit vector
   */
  std::vector<bool> GenerateBitVector(int shiftdirection = MSBFirst);

  /**
   * @brief loads the parameter configuration from an XML file
   * @param filename  - filename of the XML file to load
   * @return          - a tinyxml2 error code (or tinyxml2::XML_NO_ERROR on success)
   */
  // tinyxml2::XMLError LoadFromXMLFile(std::string filename);
  /**
   * @brief saves the current configuration in an XML file
   * @param filename      - filename to save to
   * @param devicename    - some additional information about what is saved, e.g. "Sample 2"
   * @return              - a tinyxml2 error code (or tinyxml2::XML_NO_ERROR on success)
   */
  // tinyxml2::XMLError SaveToXMLFile(std::string filename, std::string devicename = "");
private:
protected:
  std::map<int, std::string> indextoname;
  std::map<std::string, int> nametoindex;

  std::vector<std::pair<std::string, int>> parameters; // first index is the bit order and second is
  // the parameter value itself
  // in the order as it is shifted into the chip
};

#endif /* DEVICES_ATLASPIX_ATLASPIX_CONFIG_HPP_ */
