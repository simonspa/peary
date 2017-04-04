#include "../extern/cpp-readline/src/Console.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "pearycli.hpp"
#include "configuration.hpp"
#include <fstream>

using namespace caribou;
using ret = CppReadline::Console::ReturnCode;

caribou::Configuration pearycli::config = caribou::Configuration();

pearycli::pearycli() : c("# ") {

  // Register console commands
  c.registerCommand("list_devices", devices);
  c.registerCommand("add_device", addDevice);
  c.registerCommand("verbosity", verbosity);
  c.registerCommand("delay", delay);
  
  c.registerCommand("init", init);
  c.registerCommand("powerOn", powerOn);
  c.registerCommand("powerOff", powerOff);
  c.registerCommand("setVoltage", setVoltage);
  c.registerCommand("voltageOff", voltageOff);
  c.registerCommand("voltageOn", voltageOn);

  c.registerCommand("setRegister", setRegister);
  c.registerCommand("getRegister", getRegister);

  c.registerCommand("scanDAC", scanDAC);
  
  c.registerCommand("exploreInterface", exploreInterface);
  c.registerCommand("getADC", getADC);
  c.registerCommand("powerStatusLog", powerStatusLog);
}

pearycli::~pearycli() {
  // Delete the device manager
  delete manager;
}

int pearycli::devices(const std::vector<std::string> &) {

  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "ID " << i << ": " << d->getName();
      i++;
    }
  }
  catch (caribou::DeviceException &e) {}
  return ret::Ok;
}

int pearycli::addDevice(const std::vector<std::string> & input) {

  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE [DEVICE...]";
    return ret::Error;
  }

  try {
    // Spawn all devices
    for(auto d = input.begin()+1; d != input.end(); d++) {
      // ...if we have a configuration for them
      if(config.SetSection(*d)) {
	size_t device_id = manager->addDevice(*d,config);
	LOG(logINFO) << "Manager returned device ID " << device_id << ".";
      }
      else { LOG(logERROR) << "No configuration found for device " << *d; }
    }
  }
  catch (caribou::DeviceException &e) {
    LOG(logCRITICAL) << "This went wrong: " << e.what();
  }
  return ret::Ok;
}

int pearycli::verbosity(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " LOGLEVEL";
    return ret::Error;
  }
  Log::ReportingLevel() = Log::FromString(input.at(1));
  return ret::Ok;
}

int pearycli::delay(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " TIME_MS";
    return ret::Error;
  }
  mDelay(std::stoi(input.at(1)));
  return ret::Ok;
}

int pearycli::init(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->init();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}


int pearycli::powerOn(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOn();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::powerOff(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOff();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::setVoltage(const std::vector<std::string> & input) {
  if (input.size() < 4) {
    LOG(logINFO) << "Usage: " << input.at(0) << " OUTPUT_NAME OUTPUT_VALUE DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setVoltage(input.at(1),std::stod(input.at(2)));
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::voltageOn(const std::vector<std::string> & input) {
  if (input.size() < 3) {
    LOG(logINFO) << "Usage: " << input.at(0) << " OUTPUT_NAME DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(2)));
    dev->voltageOn(input.at(1));
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::voltageOff(const std::vector<std::string> & input) {
  if (input.size() < 3) {
    LOG(logINFO) << "Usage: " << input.at(0) << " OUTPUT_NAME DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(2)));
    dev->voltageOff(input.at(1));
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::setRegister(const std::vector<std::string> & input) {
  if (input.size() < 4) {
    LOG(logINFO) << "Usage: " << input.at(0) << " REGISTER_NAME REGISTER_VALUE DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setRegister(input.at(1),std::stoi(input.at(2)));
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getRegister(const std::vector<std::string> & input) {
  if (input.size() < 3) {
    LOG(logINFO) << "Usage: " << input.at(0) << " REGISTER_NAME DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(2)));
    uint32_t value = dev->getRegister(input.at(1));
    LOG(logINFO) << input.at(1) << " = " << value;
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::scanDAC(const std::vector<std::string> & input) {
  if (input.size() < 6) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DAC_NAME MIN MAX DELAY[ms] DEVICE_ID";
    return ret::Error;
  }
  
  std::map<std::string,int> output_mux_DAC {
    {"bias_disc_N", 1},{"bias_disc_P", 2},{"bias_thadj_DAC", 3},{"bias_preamp_casc", 4},{"ikrum",5},{"bias_preamp",6},{"bias_buffers_1st",7},{"bias_buffers_2st",8},{"bias_thadj_casc",9},{"bias_mirror_casc",10},{"vfbk",11},{"threshold_LSB",12},{"threshold_MSB",12},{"test_cap_2",13},{"test_cap_1_LSB",14},{"test_cap_1_MSB",14}};
  
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(5)));

    std::vector<std::pair<int,int>> data;
    
    // Set the register in output_mux_DAC:
    dev->setRegister("output_mux_DAC",output_mux_DAC[input.at(1)]);

    // Now sample through the DAC range and read the ADC at the "DAC_OUT" pin (VOL_IN_1)
    for(int i = std::stoi(input.at(2)); i < std::stoi(input.at(3)); i++) {
      dev->setRegister(input.at(1),i);
      // Wait a bit, in ms:
      mDelay(std::stoi(input.at(4)));
      // Read the ADC
      double adc = dev->getADC("DAC_OUT");
      LOG(logINFO) << "DAC " << input.at(1) << " " << i
		   << " = " << adc << "V";
      data.push_back(std::make_pair(i,adc));
    }

    // Write CSV file
    std::ofstream myfile;
    std::string filename = "dacscan_" + input.at(1) + ".csv";
    myfile.open (filename);
    myfile << "# pearycli > scanDAC\n";
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << "\n";
    myfile << "# measured voltage using ADC signal \"" << input.at(4) << "\"\n";
    myfile << "# with " << input.at(4) << "ms delay between setting register and sampling ADC.\n";
    for(auto i : data) {
      myfile << i.first << "," << i.second << "\n";
    }
    myfile << "# EOF\n";
    myfile.close();
    LOG(logINFO) << "Data writte to file: \"" << filename << "\"";
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::exploreInterface(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->exploreInterface();
  }
  catch (caribou::DeviceException & e) {
    LOG(logCRITICAL) << "Exception: " << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getADC(const std::vector<std::string> & input) {
  if (input.size() < 3) {
    LOG(logINFO) << "Usage: " << input.at(0) << " CHANNEL_ID DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Voltage: " << dev->getADC(std::stoi(input.at(1))) << "V";
  }
  catch (caribou::ConfigInvalid &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::powerStatusLog(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerStatusLog();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}
