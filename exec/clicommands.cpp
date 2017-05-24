#include <fstream>
#include "../extern/cpp-readline/src/Console.hpp"
#include "configuration.hpp"
#include "log.hpp"
#include "pearycli.hpp"
#include "utils.hpp"

using namespace caribou;
using ret = CppReadline::ReturnCode;

caribou::Configuration pearycli::config = caribou::Configuration();

pearycli::pearycli() : c("# ") {

  // Register console commands
  c.registerCommand("list_devices", devices, "Lists all registered devices", 0);
  c.registerCommand("add_device", addDevice, "Registers new device(s)", 1, "DEVICE [DEVICE...]");
  c.registerCommand("verbosity", verbosity, "Changes the logging verbosity", 1, "LOGLEVEL");
  c.registerCommand("delay", delay, "Adds a delay in Milliseconds", 1, "DELAY_MS");

  c.registerCommand("init", configure, "Initialize and configure the selected device", 1, "DEVICE_ID");
  c.registerCommand("configure", configure, "Initialize and configure the selected device", 1, "DEVICE_ID");

  c.registerCommand("powerOn", powerOn, "Power up the selected device", 1, "DEVICE_ID");
  c.registerCommand("powerOff", powerOff, "Power down the selected device", 1, "DEVICE_ID");
  c.registerCommand("setVoltage",
                    setVoltage,
                    "Set the output voltage NAME to VALUE (in V) on the selected device",
                    3,
                    "NAME VALUE DEVICE_ID");
  c.registerCommand(
    "getVoltage", getVoltage, "Get the output voltage NAME (in V) on the selected device", 2, "NAME DEVICE_ID");
  c.registerCommand(
    "getCurrent", getCurrent, "Get the output current NAME (in A) on the selected device", 2, "NAME DEVICE_ID");
  c.registerCommand("getPower", getPower, "Get the output power NAME (in W) on the selected device", 2, "NAME DEVICE_ID");

  c.registerCommand("voltageOff", voltageOff, "Turn off output voltage NAME on the selected device", 2, "NAME DEVICE_ID");
  c.registerCommand("voltageOn", voltageOn, "Turn on output voltage NAME on the selected device", 2, "NAME DEVICE_ID");

  c.registerCommand("setRegister",
                    setRegister,
                    "Set register REG_NAME to value REG_VALUE for the selected device",
                    3,
                    "REG_NAME REG_VALUE DEVICE_ID");
  c.registerCommand(
    "getRegister", getRegister, "Read the value of register REG_NAME on the selected device", 2, "REG_NAME DEVICE_ID");
  c.registerCommand("getRegisters", getRegisters, "Read the value of all registers on the selected device", 1, "DEVICE_ID");
  c.registerCommand("configureMatrix",
                    configureMatrix,
                    "Configure the pixel matrix of the selected device with the trim/mask values for all pixels provided in "
                    "the configuration file parameter FILE_NAME",
                    2,
                    "FILE_NAME DEVICE_ID");
  c.registerCommand("configurePatternGenerator",
                    configurePatternGenerator,
                    "Configure the pattern generator of the FPGA for the selected device "
                    "with the pattern provided in the configuration file FILE_NAME",
                    2,
                    "FILE_NAME DEVICE_ID");
  c.registerCommand("triggerPatternGenerator",
                    triggerPatternGenerator,
                    "Trigger the execution of the programmed pattern generator once for the selected device",
                    1,
                    "DEVICE_ID");

  c.registerCommand("scanDAC",
                    scanDAC,
                    "Scan DAC DAC_NAME from value MIN to MAX and read the voltage from the ADC after DELAY milliseconds",
                    5,
                    "DAC_NAME MIN MAX DELAY[ms] DEVICE_ID");
  c.registerCommand(
    "scanThreshold",
    scanThreshold,
    "Scan Threshold DAC DAC_NAME from value MAX down to MIN, open the shutter via the pattern generator after "
    "DELAY_PATTERN milliseconds and read back the data from the pixel "
    "matrix."
    "The sequence is repeated REPEAT times for every threshold. "
    "Data are saved in the FILE_NAME.csv file",
    7,
    "DAC_NAME MAX MIN DELAY_PATTERN[ms] REPEAT FILE_NAME DEVICE_ID");

  c.registerCommand(
    "exploreInterface", exploreInterface, "Perform an interface communication test on the selected devce", 1, "DEVICE_ID");
  c.registerCommand("getADC",
                    getADC,
                    "Read the voltage from ADC channel CHANNEL_ID via the selected device",
                    2,
                    "CHANNEL_ID[1:8] DEVICE_ID");
  c.registerCommand(
    "powerStatusLog", powerStatusLog, "Perform a power and current measurement for the selected device", 1, "DEVICE_ID");
  c.registerCommand("daqStart", daqStart, "Start DAQ for the selected device", 1, "DEVICE_ID");
  c.registerCommand("daqStop", daqStop, "Stop DAQ for the selected device", 1, "DEVICE_ID");
  c.registerCommand("getRawData", getRawData, "Retrieve raw data from the selected device", 1, "DEVICE_ID");
  c.registerCommand("getData", getData, "Retrieve decoded data from the selected device", 1, "DEVICE_ID");
  c.registerCommand("flushMatrix", flushMatrix, "Retrieve data from the selected device and discard it", 1, "DEVICE_ID");
}

pearycli::~pearycli() {
  // Delete the device manager
  delete manager;
}

int pearycli::devices(const std::vector<std::string>&) {

  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "ID " << i << ": " << d->getName();
      i++;
    }
  } catch(caribou::DeviceException& e) {
  }
  return ret::Ok;
}

int pearycli::addDevice(const std::vector<std::string>& input) {
  try {
    // Spawn all devices
    for(auto d = input.begin() + 1; d != input.end(); d++) {
      // ...if we have a configuration for them
      if(config.SetSection(*d)) {
        size_t device_id = manager->addDevice(*d, config);
        LOG(logINFO) << "Manager returned device ID " << device_id << ".";
      } else {
        LOG(logERROR) << "No configuration found for device " << *d;
      }
    }
  } catch(caribou::DeviceException& e) {
    LOG(logCRITICAL) << "This went wrong: " << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::verbosity(const std::vector<std::string>& input) {
  Log::ReportingLevel() = Log::FromString(input.at(1));
  return ret::Ok;
}

int pearycli::delay(const std::vector<std::string>& input) {
  mDelay(std::stoi(input.at(1)));
  return ret::Ok;
}

int pearycli::configure(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->configure();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::powerOn(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOn();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::powerOff(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOff();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::setVoltage(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setVoltage(input.at(1), std::stod(input.at(2)));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getVoltage(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Voltage " << input.at(1) << "=" << dev->getVoltage(input.at(1)) << "V";
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getCurrent(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Current " << input.at(1) << "=" << dev->getCurrent(input.at(1)) << "A";
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getPower(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Power " << input.at(1) << "=" << dev->getVoltage(input.at(1)) << "W";
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::voltageOn(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->voltageOn(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::voltageOff(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->voltageOff(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::setRegister(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setRegister(input.at(1), std::stoi(input.at(2)));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getRegister(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    uint32_t value = dev->getRegister(input.at(1));
    LOG(logINFO) << input.at(1) << " = " << value;
  } catch(caribou::NoDataAvailable& e) {
    LOG(logWARNING) << e.what();
    return ret::Ok;
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getRegisters(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    std::vector<std::pair<std::string, uint32_t>> regvalues = dev->getRegisters();
    for(auto& i : regvalues) {
      LOG(logINFO) << i.first << " = " << i.second;
    }
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::configureMatrix(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->configureMatrix(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::configurePatternGenerator(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->configurePatternGenerator(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::triggerPatternGenerator(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->triggerPatternGenerator();
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::scanDAC(const std::vector<std::string>& input) {
  std::map<std::string, int> output_mux_DAC{{"bias_disc_n", 1},
                                            {"bias_disc_p", 2},
                                            {"bias_thadj_dac", 3},
                                            {"bias_preamp_casc", 4},
                                            {"ikrum", 5},
                                            {"bias_preamp", 6},
                                            {"bias_buffers_1st", 7},
                                            {"bias_buffers_2st", 8},
                                            {"bias_thadj_casc", 9},
                                            {"bias_mirror_casc", 10},
                                            {"vfbk", 11},
                                            {"threshold_lsb", 12},
                                            {"threshold_msb", 12},
                                            {"test_cap_2", 13},
                                            {"test_cap_1_lsb", 14},
                                            {"test_cap_1_msb", 14}};

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(5)));

    std::vector<std::pair<int, double>> data;

    // Set the register in output_mux_DAC:
    std::string dacname = input.at(1);
    std::transform(dacname.begin(), dacname.end(), dacname.begin(), ::tolower);
    dev->setRegister("output_mux_DAC", output_mux_DAC[dacname]);

    if(std::stoi(input.at(2)) > std::stoi(input.at(3))) {
      LOG(logERROR) << "Range invalid";
      return ret::Error;
    }

    // Now sample through the DAC range and read the ADC at the "DAC_OUT" pin (VOL_IN_1)
    for(int i = std::stoi(input.at(2)); i <= std::stoi(input.at(3)); i++) {
      dev->setRegister(input.at(1), i);
      // Wait a bit, in ms:
      mDelay(std::stoi(input.at(4)));
      // Read the ADC
      double adc = dev->getADC("DAC_OUT");
      LOG(logINFO) << "DAC " << input.at(1) << " " << i << " = " << adc << "V";
      data.push_back(std::make_pair(i, adc));
    }

    // Write CSV file
    std::ofstream myfile;
    std::string filename = "dacscan_" + input.at(1) + ".csv";
    myfile.open(filename);
    myfile << "# pearycli > scanDAC\n";
    myfile << "# Timestamp: " << LOGTIME << "\n";
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << "\n";
    myfile << "# with " << input.at(4) << "ms delay between setting register and ADC sampling.\n";
    for(auto i : data) {
      myfile << i.first << "," << i.second << "\n";
    }
    myfile.close();
    LOG(logINFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::exploreInterface(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->exploreInterface();
  } catch(caribou::DeviceException& e) {
    LOG(logCRITICAL) << "Exception: " << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getADC(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Voltage: " << dev->getADC(std::stoi(input.at(1))) << "V";
  } catch(caribou::ConfigInvalid&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::powerStatusLog(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerStatusLog();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::daqStart(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->daqStart();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::daqStop(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->daqStop();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getRawData(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->getRawData();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getData(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    pearydata data = dev->getData();
    for(auto& px : data) {
      LOG(logINFO) << px.first.first << "|" << px.first.second << " : " << *px.second;
    }
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::flushMatrix(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    pearydata data = dev->getData();
  } catch(caribou::DeviceException&) {
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::scanThreshold(const std::vector<std::string>& input) {

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(7)));

    std::ofstream myfile;
    std::string filename = input.at(6) + ".csv";
    myfile.open(filename);
    myfile << "# pearycli > scanThreshold\n";
    myfile << "# Software version: " << dev->getVersion() << "\n";
    myfile << "# Firmware version: " << dev->getFirmwareVersion() << "\n";
    myfile << "# Register state: " << listVector(dev->getRegisters()) << "\n";
    myfile << "# Timestamp: " << LOGTIME << "\n";
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << ", " << input.at(5)
           << " times\n";
    myfile << "# with " << input.at(4) << "ms delay between setting register and reading matrix.\n";

    if(std::stoi(input.at(2)) < std::stoi(input.at(3))) {
      LOG(logERROR) << "Range invalid";
      return ret::Error;
    }

    // Sample through the DAC range, trigger the PG and read back the data
    for(int i = std::stoi(input.at(2)); i >= std::stoi(input.at(3)); i--) {
      LOG(logINFO) << "Threshold " << i;
      dev->setRegister(input.at(1), i);

      std::stringstream responses;
      responses << "Pixel responses: ";
      for(int j = 0; j < std::stoi(input.at(5)); j++) {
        // Wait a bit, in ms:
        mDelay(std::stoi(input.at(4)));
        // Send pattern:
        dev->triggerPatternGenerator();
        // Read the data:
        pearydata frame = dev->getData();
        for(auto& px : frame) {
          myfile << i << "," << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
        }
        responses << frame.size() << " ";
        mDelay(std::stoi(input.at(4)));
      }
      LOG(logINFO) << responses.str();
    }

    LOG(logINFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}
