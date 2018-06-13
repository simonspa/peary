#include <fstream>
#include "Console.hpp"
#include "configuration.hpp"
#include "log.hpp"
#include "pearycli.hpp"
#include "utils.hpp"

using namespace caribou;

caribou::Configuration pearycli::config = caribou::Configuration();

pearycli::pearycli() : Console("# ") {

  // Register console commands
  registerCommand("list_devices", devices, "Lists all registered devices", 0);
  registerCommand("add_device", addDevice, "Registers new device(s)", 1, "DEVICE [DEVICE...]");
  registerCommand("verbosity", verbosity, "Changes the logging verbosity", 1, "LOGLEVEL");
  registerCommand("delay", delay, "Adds a delay in Milliseconds", 1, "DELAY_MS");

  registerCommand(
    "list_commands", list_commands, "list available device-specific commands for selected device", 1, "DEVICE_ID");

  registerCommand("version", version, "Print software and firmware version of the selected device", 1, "DEVICE_ID");
  registerCommand("init", configure, "Initialize and configure the selected device", 1, "DEVICE_ID");
  registerCommand("configure", configure, "Initialize and configure the selected device", 1, "DEVICE_ID");
  registerCommand("reset", reset, "Send reset signal to the selected device", 1, "DEVICE_ID");

  registerCommand("powerOn", powerOn, "Power up the selected device", 1, "DEVICE_ID");
  registerCommand("powerOff", powerOff, "Power down the selected device", 1, "DEVICE_ID");
  registerCommand("setVoltage",
                  setVoltage,
                  "Set the output voltage NAME to VALUE (in V) on the selected device",
                  3,
                  "NAME VALUE DEVICE_ID");
  registerCommand("setBias",
                  setBias,
                  "Set the output bias voltage NAME to VALUE (in V) on the selected device",
                  3,
                  "NAME VALUE DEVICE_ID");
  registerCommand(
    "setCurrent",
    setCurrent,
    "Set the current for current source NAME to VALUE (in uA) with polarity POL (0 = PULL, 1 = PUSH) on the selected device",
    4,
    "NAME VALUE POL DEVICE_ID");
  registerCommand(
    "getVoltage", getVoltage, "Get the output voltage NAME (in V) on the selected device", 2, "NAME DEVICE_ID");
  registerCommand(
    "getCurrent", getCurrent, "Get the output current NAME (in A) on the selected device", 2, "NAME DEVICE_ID");
  registerCommand("getPower", getPower, "Get the output power NAME (in W) on the selected device", 2, "NAME DEVICE_ID");

  registerCommand("voltageOff", switchOff, "Turn off output voltage NAME on the selected device", 2, "NAME DEVICE_ID");
  registerCommand("voltageOn", switchOn, "Turn on output voltage NAME on the selected device", 2, "NAME DEVICE_ID");
  registerCommand("switchOn",
                  switchOn,
                  "Switch on the periphery component identified by NAME and controlled by the selected device",
                  2,
                  "NAME DEVICE_ID");
  registerCommand("switchOff",
                  switchOff,
                  "Switch off the periphery component identified by NAME and controlled by the selected device",
                  2,
                  "NAME DEVICE_ID");
  registerCommand("biasOff", switchOff, "Turn off bias voltage NAME on the selected device", 2, "NAME DEVICE_ID");
  registerCommand("biasOn", switchOn, "Turn on bias voltage NAME on the selected device", 2, "NAME DEVICE_ID");

  registerCommand("setRegister",
                  setRegister,
                  "Set register REG_NAME to value REG_VALUE for the selected device",
                  3,
                  "REG_NAME REG_VALUE DEVICE_ID");
  registerCommand(
    "getRegister", getRegister, "Read the value of register REG_NAME on the selected device", 2, "REG_NAME DEVICE_ID");
  registerCommand("getRegisters", getRegisters, "Read the value of all registers on the selected device", 1, "DEVICE_ID");

  registerCommand("scanDAC",
                  scanDAC,
                  "Scan DAC DAC_NAME from value MIN to MAX and read the voltage from the ADC after DELAY milliseconds. The "
                  "sequence is repeated REPEAT times for every DAC setting. Data are saved in the FILE_NAME.csv file",
                  7,
                  "DAC_NAME MIN MAX DELAY[ms] REPEAT FILE_NAME DEVICE_ID");
  registerCommand("scanDAC2D",
                  scanDAC2D,
                  "For each value of DAC1_NAME between DAC1_MIN and DAC1_MAX, scan DAC DAC2_NAME from value DAC2_MIN to "
                  "DAC2_MAX and read the voltage from the ADC after DELAY milliseconds. The sequence is repeated REPEAT "
                  "times for every DAC setting. Data are saved in the FILE_NAME.csv file",
                  10,
                  "DAC1_NAME DAC1_MIN DAC1_MAX DAC2_NAME DAC2_MIN DAC2_MAX DELAY[ms] REPEAT FILE_NAME DEVICE_ID");
  registerCommand(
    "scanThreshold",
    scanThreshold,
    "Scan Threshold DAC DAC_NAME from value MAX down to MIN on DEVICE_ID1, open the shutter via the pattern generator after "
    "DELAY_PATTERN milliseconds and read back the data from the pixel matrix of DEVICE_ID2. The sequence is repeated REPEAT "
    "times for every threshold. Data are saved in the FILE_NAME.csv file.\nOPTIONAL: If the two additional arguments are "
    "provided, always write to register REG on DEVICE_ID_3 after starting the pattern generator.",
    8,
    "DAC_NAME MAX MIN DEVICE_ID1 DELAY_PATTERN[ms] REPEAT FILE_NAME DEVICE_ID2 [REG DEVICE_ID_3]");
  registerCommand(
    "scanThreshold2D",
    scanThreshold2D,
    "For each value of DAC1_NAME between DAC1_MIN and DAC1_MAX on DEVICE_ID1, scan DAC2_NAME from value DAC2_MAX down to "
    "DAC2_MIN on DEVICE_ID2, open the shutter on DEVICE_ID2 via the pattern generator after DELAY_PATTERN milliseconds and "
    "read back the data from the pixel matrix. The sequence is repeated REPEAT times for every setting. Data are saved in "
    "the FILE_NAME.csv file",
    10,
    "DAC1_NAME DAC1_MAX DAC1_MIN DEVICE_ID1 DAC2_NAME DAC2_MAX DAC2_MIN DEVICE_ID2 DELAY_PATTERN[ms] REPEAT FILE_NAME");

  registerCommand("getADC",
                  getADC,
                  "Read the voltage from ADC channel CHANNEL_ID via the selected device",
                  2,
                  "CHANNEL_ID[1:8] DEVICE_ID");
  registerCommand("daqStart", daqStart, "Start DAQ for the selected device", 1, "DEVICE_ID");
  registerCommand("daqStop", daqStop, "Stop DAQ for the selected device", 1, "DEVICE_ID");
  registerCommand("getRawData", getRawData, "Retrieve raw data from the selected device", 1, "DEVICE_ID");
  registerCommand("getData", getData, "Retrieve decoded data from the selected device.", 1, "DEVICE_ID");

  registerCommand("dataTuning",
                  dataTuning,
                  "Tune using data from the selected device. Usage : dataTuning vmax nsteps npulses deviceid",
                  4,
                  "DEVICE_ID");
  registerCommand("VerifyTuning",
                  VerifyTuning,
                  "Tune using data from the selected device. Usage : dataTuning vmax nsteps npulses TDACFilePath deviceid",
                  5,
                  "DEVICE_ID");

  registerCommand(
    "acquire",
    acquire,
    "Acquire NUM events/frames from the selected device (1). For every event/frame, the pattern generator is "
    "triggered once and a readout of the device is attempted. Prints all pixel hits if LONG is set to 1, else "
    "just the number of pixel responses.\nOPTIONAL: If the two additional arguments are provided, always write "
    "to register REG on DEVICE_ID_2 after starting the pattern generator.",
    4,
    "NUM LONG[0/1] FILENAME DEVICE_ID_1 [REG DEVICE_ID_2]");
  registerCommand("flushMatrix", flushMatrix, "Retrieve data from the selected device and discard it", 1, "DEVICE_ID");

  registerCommand("lock", lock, "lock the device", 1, "DEVICE_ID");
  registerCommand("unlock", unlock, "unlock the device", 1, "DEVICE_ID");
  registerCommand(
    "setThreshold", setThreshold, "Setting threshold, usage setThreshold Threshold(V) device ID", 2, "DEVICE_ID");
  registerCommand("setVMinus", setVMinus, "Setting VMinusPix, usage setVMinus VMinus(V) device ID", 2, "DEVICE_ID");
  registerCommand(
    "getTriggerCount", getTriggerCount, "return current trigger count, usage getTriggerCountdevice ID", 1, "DEVICE_ID");
  registerCommand("pulse", pulse, "pulse, usage pulse npulse nup ndown amplitude device ID", 5, "DEVICE_ID");
  registerCommand("setPixelInjection",
                  SetPixelInjection,
                  "Set pixel injection and output, usage : setPixelInjection col row analog_output state hitbus state "
                  "injection state device id",
                  6,
                  "DEVICE_ID");
  registerCommand("doSCurve",
                  doSCurve,
                  "Perform a s-curve measurement, usage doSCurve col row vmin vmax npulses npoints device_id",
                  7,
                  "DEVICE_ID");
  registerCommand("doSCurves",
                  doSCurves,
                  "Perform  s-curve measurement for the whole matrix, usage doSCurves vmin vmax npulses npoints device_id",
                  5,
                  "DEVICE_ID");
  registerCommand("doNoiseCurve",
                  doNoiseCurve,
                  "determine the noise floor for a given pixel, usage : doNoiseCurve col row",
                  3,
                  "DEVICE_ID");
  registerCommand(
    "setAllTDAC", setAllTDAC, "Setting TDAC for the whole matrix, usage setAllTDAC value device ID", 2, "DEVICE_ID");
  registerCommand(
    "LoadTDAC", LoadTDAC, "Setting TDAC for the whole matrix, usage LoadTDAC filename device ID", 2, "DEVICE_ID");
  registerCommand(
    "LoadConfig", LoadConfig, "Load Config for the whole matrix, usage LoadConfig basename device ID", 2, "DEVICE_ID");
  registerCommand(
    "WriteConfig", WriteConfig, "Write Config for the whole matrix, usage WriteConfig basename device ID", 2, "DEVICE_ID");
  registerCommand("TDACScan",
                  TDACScan,
                  "TDAC Scan for a given VNDAC for the whole matrix, usage TDACScan basefolder VNDAC TDACSteps vmin vmax "
                  "npulses npoints device ID",
                  8,
                  "DEVICE_ID");
  registerCommand("SetMatrix",
                  SetMatrix,
                  "Set Matrix associated to this device, usage WriteConfig matrix(M1,M2,M1ISO) device ID",
                  2,
                  "DEVICE_ID");
  registerCommand("MaskPixel", MaskPixel, "mask a pixel in the matrix, usage MaskPixel col row device ID", 3, "DEVICE_ID");
  registerCommand("isLocked", isLocked, "return the lock status of the receiver", 1, "DEVICE_ID");
}

pearycli::~pearycli() {
  // Delete the device manager
  delete manager;
}

void pearycli::listUnregisteredCommands() {
  std::cout << "Additional commands registered by individual devices:\n";
  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      std::cout << "ID " << i << " - " << d->getName() << ":" << std::endl;
      for(auto& cmd : d->list_commands()) {
        std::cout << "\t" << cmd.first << std::endl;
      }
      i++;
    }
  } catch(caribou::caribouException& e) {
  }
}

int pearycli::unregisteredCommand(const std::vector<std::string>& input) {

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.back()));

    auto commands = dev->list_commands();

    auto value = input.front();
    auto cmd = std::find_if(commands.begin(), commands.end(), [&value](std::pair<std::string, std::size_t> const& elem) {
      return elem.first == value;
    });

    // Command not found
    if(cmd == commands.end()) {
      throw std::invalid_argument("Command '" + input.front() + "' for device " + dev->getName() + " not found.\n");
    }

    const std::vector<std::string> args(input.begin() + 1, input.end() - 1);
    dev->command(input.front(), args);
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

std::string pearycli::allDeviceParameters() {

  std::stringstream responses;
  responses << "\n";

  std::vector<caribouDevice*> devs = manager->getDevices();
  for(auto d : devs) {
    responses << "# " << d->getName() << ": " << listVector(d->getRegisters()) << "\n";
  }
  return responses.str();
}

std::string pearycli::getFileHeader(std::string function, caribouDevice* dev) {

  std::stringstream header;

  header << "# pearycli > " << function << "\n";
  header << "# Software version: " << dev->getVersion() << "\n";
  header << "# Firmware version: " << dev->getFirmwareVersion() << "\n";
  header << "# Register state: " << allDeviceParameters();
  header << "# Timestamp: " << LOGTIME << "\n";

  return header.str();
}

int pearycli::devices(const std::vector<std::string>&) {

  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(INFO) << "ID " << i << ": " << d->getName();
      i++;
    }
  } catch(caribou::DeviceException& e) {
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::addDevice(const std::vector<std::string>& input) {
  try {
    // Spawn all devices
    for(auto d = input.begin() + 1; d != input.end(); d++) {
      // ...if we have a configuration for them
      if(config.SetSection(*d)) {
        size_t device_id = manager->addDevice(*d, config);
        LOG(INFO) << "Manager returned device ID " << device_id << ".";
      } else {
        LOG(ERROR) << "No configuration found for device " << *d;
      }
    }
  } catch(caribou::caribouException& e) {
    LOG(FATAL) << "This went wrong: " << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::verbosity(const std::vector<std::string>& input) {
  Log::setReportingLevel(Log::getLevelFromString(input.at(1)));
  return ReturnCode::Ok;
}

int pearycli::delay(const std::vector<std::string>& input) {
  mDelay(std::stoi(input.at(1)));
  return ReturnCode::Ok;
}

int pearycli::list_commands(const std::vector<std::string>& input) {

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    std::cout << "List of commands available for device ID " << input.at(1) << " - " << dev->getName() << ":" << std::endl;
    for(auto& cmd : dev->list_commands()) {
      std::cout << "\t" << cmd.first << " (args: " << cmd.second << ")" << std::endl;
    }
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::version(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    LOG(STATUS) << dev->getVersion();
    LOG(STATUS) << dev->getFirmwareVersion();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::configure(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->configure();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::reset(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->reset();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::powerOn(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOn();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::powerOff(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOff();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setVoltage(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setVoltage(input.at(1), std::stod(input.at(2)));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setBias(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setBias(input.at(1), std::stod(input.at(2)));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setCurrent(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(4)));
    dev->setCurrent(input.at(1), std::stoi(input.at(2)), static_cast<bool>(std::stoi(input.at(3))));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getVoltage(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(INFO) << "Voltage " << input.at(1) << "=" << dev->getVoltage(input.at(1)) << "V";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getCurrent(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(INFO) << "Current " << input.at(1) << "=" << dev->getCurrent(input.at(1)) << "A";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getPower(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(INFO) << "Power " << input.at(1) << "=" << dev->getVoltage(input.at(1)) << "W";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::switchOn(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->switchOn(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::switchOff(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->switchOff(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setRegister(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setRegister(input.at(1), std::stoi(input.at(2)));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getRegister(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    uint32_t value = dev->getRegister(input.at(1));
    LOG(INFO) << input.at(1) << " = " << value;
  } catch(caribou::NoDataAvailable& e) {
    LOG(WARNING) << e.what();
    return ReturnCode::Ok;
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getRegisters(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    std::vector<std::pair<std::string, uint32_t>> regvalues = dev->getRegisters();
    for(auto& i : regvalues) {
      LOG(INFO) << i.first << " = " << i.second;
    }
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
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
                                            {"threshold", 12},
                                            {"threshold_lsb", 12},
                                            {"threshold_msb", 12},
                                            {"test_cap_2", 13},
                                            {"test_cap_1_lsb", 14},
                                            {"test_cap_1_msb", 14}};

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(7)));

    std::vector<std::pair<int, double>> data;

    // Set the register in output_mux_DAC:
    std::string dacname = input.at(1);
    std::transform(dacname.begin(), dacname.end(), dacname.begin(), ::tolower);
    dev->setRegister("output_mux_DAC", output_mux_DAC[dacname]);

    if(std::stoi(input.at(2)) > std::stoi(input.at(3))) {
      LOG(ERROR) << "Range invalid";
      return ReturnCode::Error;
    }

    uint32_t dac = 0;
    try {
      // Store the old setting of the DAC:
      dac = dev->getRegister(input.at(1));
    } catch(caribou::RegisterTypeMismatch&) {
    }

    // Now sample through the DAC range and read the ADC at the "DAC_OUT" pin (VOL_IN_1)
    for(int i = std::stoi(input.at(2)); i <= std::stoi(input.at(3)); i++) {

      std::stringstream responses;
      responses << input.at(1) << " " << i << " = ";
      for(int j = 0; j < std::stoi(input.at(5)); j++) {
        dev->setRegister(input.at(1), i);
        // Wait a bit, in ms:
        mDelay(std::stoi(input.at(4)));
        // Read the ADC
        double adc = dev->getADC("DAC_OUT");
        responses << adc << "V ";
        data.push_back(std::make_pair(i, adc));
      }
      LOG(INFO) << responses.str();
    }

    // Restore the old setting of the DAC:
    dev->setRegister(input.at(1), dac);

    // Write CSV file
    std::ofstream myfile;
    std::string filename = input.at(6) + ".csv";
    myfile.open(filename);
    myfile << getFileHeader(input.at(0), dev);
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << ", " << input.at(5)
           << " times\n";
    myfile << "# with " << input.at(4) << "ms delay between setting register and ADC sampling.\n";
    for(auto i : data) {
      myfile << i.first << "," << i.second << "\n";
    }
    myfile.close();
    LOG(INFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::scanDAC2D(const std::vector<std::string>& input) {
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
                                            {"threshold", 12},
                                            {"threshold_lsb", 12},
                                            {"threshold_msb", 12},
                                            {"test_cap_2", 13},
                                            {"test_cap_1_lsb", 14},
                                            {"test_cap_1_msb", 14}};

  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(10)));

    std::vector<std::pair<std::pair<int, int>, double>> data;

    // Set the register in output_mux_DAC:
    std::string dacname = input.at(1);
    std::transform(dacname.begin(), dacname.end(), dacname.begin(), ::tolower);
    dev->setRegister("output_mux_DAC", output_mux_DAC[dacname]);

    if(std::stoi(input.at(2)) > std::stoi(input.at(3)) || std::stoi(input.at(5)) > std::stoi(input.at(6))) {
      LOG(ERROR) << "Range invalid";
      return ReturnCode::Error;
    }

    uint32_t dac1 = 0;
    uint32_t dac2 = 0;
    // Store the old setting of the DAC:
    try {
      dac1 = dev->getRegister(input.at(1));
    } catch(caribou::RegisterTypeMismatch&) {
    }
    try {
      dac2 = dev->getRegister(input.at(4));
    } catch(caribou::RegisterTypeMismatch&) {
    }

    // Sample through DAC1
    for(int j = std::stoi(input.at(5)); j <= std::stoi(input.at(6)); j++) {
      LOG(INFO) << input.at(4) << ": " << j;
      dev->setRegister(input.at(4), j);

      // Now sample through the DAC2 range and read the ADC at the "DAC_OUT" pin (VOL_IN_1)
      for(int i = std::stoi(input.at(2)); i <= std::stoi(input.at(3)); i++) {
        dev->setRegister(input.at(1), i);

        std::stringstream responses;
        responses << input.at(1) << " " << i << " = ";
        for(int k = 0; k < std::stoi(input.at(8)); k++) {
          // Wait a bit, in ms:
          mDelay(std::stoi(input.at(7)));
          // Read the ADC
          double adc = dev->getADC("DAC_OUT");
          responses << adc << "V ";
          data.push_back(std::make_pair(std::make_pair(j, i), adc));
        }
        LOG(INFO) << responses.str();
      }
    }

    // Restore the old setting of the DAC:
    dev->setRegister(input.at(1), dac1);
    dev->setRegister(input.at(4), dac2);

    // Write CSV file
    std::ofstream myfile;
    std::string filename = input.at(9) + ".csv";
    myfile.open(filename);
    myfile << getFileHeader(input.at(0), dev);
    myfile << "# scanned DACs \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << " and \""
           << input.at(4) << "\", range " << input.at(5) << "-" << input.at(6) << ", " << input.at(7) << " times\n";
    myfile << "# with " << input.at(8) << "ms delay between setting register and ADC sampling.\n";
    for(auto i : data) {
      myfile << i.first.first << "," << i.first.second << "," << i.second << "\n";
    }
    myfile.close();
    LOG(INFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getADC(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(INFO) << "Voltage: " << dev->getADC(std::stoi(input.at(1))) << "V";
  } catch(caribou::ConfigInvalid&) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setOutputDirectory(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->setOutputDirectory(input.at(1));
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::daqStart(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->daqStart();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::daqStop(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->daqStop();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getRawData(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    std::vector<uint32_t> rawdata = dev->getRawData();
    LOG(INFO) << listVector(rawdata, ", ", true);
  } catch(caribou::DataException& e) {
    LOG(ERROR) << e.what();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getData(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    pearydata data = dev->getData();
    for(auto& px : data) {
      LOG(INFO) << px.first.first << "|" << px.first.second << " : " << *px.second;
    }
  } catch(caribou::DataException& e) {
    LOG(ERROR) << e.what();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::dataTuning(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(4)));
    dev->dataTuning(std::stof(input.at(1)), std::stoi(input.at(2)), std::stoi(input.at(3)));
  } catch(caribou::DataException& e) {
    LOG(ERROR) << e.what();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::VerifyTuning(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(5)));
    dev->VerifyTuning(std::stof(input.at(1)), std::stoi(input.at(2)), std::stoi(input.at(3)), input.at(4));
  } catch(caribou::DataException& e) {
    LOG(ERROR) << e.what();
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::acquire(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(4)));

    std::ofstream myfile;
    std::string filename = input.at(3) + ".csv";
    myfile.open(filename);
    myfile << getFileHeader(input.at(0), dev);

    bool testpulses = false;
    bool tp_status = false;
    caribouDevice* dev2 = nullptr;
    // Only with optional arguments provided:
    if(input.size() == 7) {
      dev2 = manager->getDevice(std::stoi(input.at(6)));
      testpulses = true;
      // Get status of the register to toggle:
      tp_status = static_cast<bool>(dev2->getRegister(input.at(5)));
    }

    for(int n = 0; n < std::stoi(input.at(1)); n++) {
      try {
        pearydata data;
        try {
          if(testpulses) {
            // Send pattern:
            dev->command("triggerPatternGenerator", "0");
            // Trigger DEV2, toggle to NOT(tp_status)
            dev2->setRegister(input.at(5), static_cast<int>(!tp_status));
          } else {
            // Send pattern:
            dev->command("triggerPatternGenerator", "1");
          }
          // Wait
          mDelay(100);
          // Read the data:
          data = dev->getData();
          // Reset DEV2 testpulse to tp_status
          if(testpulses) {
            dev2->setRegister(input.at(5), static_cast<int>(tp_status));
          }
        } catch(caribou::DataException& e) {
          // Retrieval failed, retry once more before aborting:
          LOG(WARNING) << e.what() << ", retyring once.";
          mDelay(10);
          data = dev->getData();
        }

        if(std::stoi(input.at(2))) {
          LOG(INFO) << "===== " << n << " =====";
          for(auto& px : data) {
            LOG(INFO) << px.first.first << "|" << px.first.second << " : " << *px.second;
          }
        } else {
          LOG(INFO) << n << " | " << data.size() << " pixel responses";
        }
        myfile << "===== " << n << " =====\n";
        for(auto& px : data) {
          myfile << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
        }

      } catch(caribou::DataException& e) {
        continue;
      }
    }
  } catch(caribou::caribouException& e) {
    LOG(FATAL) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::flushMatrix(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    pearydata data = dev->getData();
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::lock(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->lock();
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::unlock(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->unlock();
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}
int pearycli::pulse(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(5)));
    dev->pulse(std::stoi(input.at(1)), std::stoi(input.at(2)), std::stoi(input.at(3)), std::stof(input.at(4)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setThreshold(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->setThreshold(std::stof(input.at(1)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setVMinus(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->setVMinus(std::stof(input.at(1)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::SetPixelInjection(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(6)));
    dev->SetPixelInjection(std::stoi(input.at(1)),
                           std::stoi(input.at(2)),
                           std::stoi(input.at(3)),
                           std::stoi(input.at(4)),
                           std::stoi(input.at(5)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::doNoiseCurve(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->doNoiseCurve(std::stoi(input.at(1)), std::stoi(input.at(2)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::doSCurve(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(7)));
    std::cout << std::stoi(input.at(7)) << std::endl;
    dev->doSCurve(std::stoi(input.at(1)),
                  std::stoi(input.at(2)),
                  std::stof(input.at(3)),
                  std::stof(input.at(4)),
                  std::stoi(input.at(5)),
                  std::stoi(input.at(6)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::doSCurves(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(5)));
    dev->doSCurves(std::stof(input.at(1)), std::stof(input.at(2)), std::stoi(input.at(3)), std::stoi(input.at(4)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::setAllTDAC(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->setAllTDAC(std::stof(input.at(1)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::LoadTDAC(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->LoadTDAC(input.at(1));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::LoadConfig(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->LoadConfig(input.at(1));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::WriteConfig(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->WriteConfig(input.at(1));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::TDACScan(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(8)));

    dev->TDACScan(input.at(1),
                  std::stoi(input.at(2)),
                  std::stoi(input.at(3)),
                  std::stof(input.at(4)),
                  std::stof(input.at(5)),
                  std::stoi(input.at(6)),
                  std::stoi(input.at(7)));

  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::SetMatrix(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(2)));
    dev->SetMatrix(input.at(1));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::MaskPixel(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(3)));
    dev->MaskPixel(std::stoi(input.at(1)), std::stoi(input.at(2)));
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::isLocked(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->isLocked();
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::getTriggerCount(const std::vector<std::string>& input) {
  try {
    caribouDevice* dev = manager->getDevice(std::stoi(input.at(1)));
    dev->getTriggerCount();
  } catch(caribou::caribouException& e) {
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::scanThreshold(const std::vector<std::string>& input) {

  try {
    caribouDevice* dev1 = manager->getDevice(std::stoi(input.at(4)));
    caribouDevice* dev2 = manager->getDevice(std::stoi(input.at(8)));
    caribouDevice* dev3 = NULL;

    // Only with optional arguments provided:
    bool testpulses = false;
    if(input.size() == 11) {
      dev3 = manager->getDevice(std::stoi(input.at(10)));
      testpulses = true;
    }

    std::ofstream myfile;
    std::string filename = input.at(7) + ".csv";
    myfile.open(filename);
    myfile << getFileHeader(input.at(0), dev2);
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << ", " << input.at(6)
           << " times\n";
    myfile << "# with " << input.at(5) << "ms delay between setting register and reading matrix.\n";

    if(std::stoi(input.at(2)) < std::stoi(input.at(3))) {
      LOG(ERROR) << "Range invalid";
      return ReturnCode::Error;
    }

    // Store the old setting of the DAC if possible:
    uint32_t dac = 0;
    bool dac_cached = false;
    try {
      dac = dev1->getRegister(input.at(1));
      dac_cached = true;
    } catch(caribou::RegisterTypeMismatch&) {
    }

    // Sample through the DAC range, trigger the PG and read back the data
    for(int i = std::stoi(input.at(2)); i >= std::stoi(input.at(3)); i--) {
      LOG(INFO) << input.at(1) << " = " << i;
      dev1->setRegister(input.at(1), i);

      std::stringstream responses;
      responses << "Pixel responses: ";
      for(int j = 0; j < std::stoi(input.at(6)); j++) {
        // Wait a bit, in ms:
        mDelay(std::stoi(input.at(5)));

        pearydata frame;
        try {
          // Send pattern:
          if(!testpulses)
            dev2->command("triggerPatternGenerator", "1");
          else {
            dev2->command("triggerPatternGenerator", "0");
            dev3->setRegister(input.at(9), 1);
            mDelay(10);
          }

          // Read the data:
          frame = dev2->getData();
          if(testpulses)
            dev3->setRegister(input.at(9), 0);
        } catch(caribou::DataException& e) {
          // Retrieval failed, retry once more before aborting:
          LOG(WARNING) << e.what() << ", retyring once.";
          mDelay(10);
          frame = dev2->getData();
        }

        for(auto& px : frame) {
          myfile << i << "," << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
        }
        responses << frame.size() << " ";
        mDelay(std::stoi(input.at(5)));
      }
      LOG(INFO) << responses.str();
    }

    // Restore the old setting of the DAC:
    if(dac_cached) {
      dev1->setRegister(input.at(1), dac);
    }

    LOG(INFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}

int pearycli::scanThreshold2D(const std::vector<std::string>& input) {

  try {
    caribouDevice* dev1 = manager->getDevice(std::stoi(input.at(4)));
    caribouDevice* dev2 = manager->getDevice(std::stoi(input.at(8)));

    std::ofstream myfile;
    std::string filename = input.at(11) + ".csv";
    myfile.open(filename);
    myfile << getFileHeader(input.at(0), dev1);
    myfile << getFileHeader(input.at(0), dev2);
    myfile << "# scanned DAC \"" << input.at(1) << "\", range " << input.at(2) << "-" << input.at(3) << " and DAC \""
           << input.at(5) << "\", range " << input.at(6) << "-" << input.at(7) << ", " << input.at(10) << " times\n";
    myfile << "# with " << input.at(9) << "ms delay between setting register and reading matrix.\n";

    if(std::stoi(input.at(2)) < std::stoi(input.at(3))) {
      LOG(ERROR) << "Range invalid";
      return ReturnCode::Error;
    }
    if(std::stoi(input.at(6)) < std::stoi(input.at(7))) {
      LOG(ERROR) << "Range invalid";
      return ReturnCode::Error;
    }

    // Store the old setting of the DAC if possible:
    uint32_t dac1 = 0;
    bool dac1_cached = false;
    uint32_t dac2 = 0;
    bool dac2_cached = false;
    try {
      dac1 = dev1->getRegister(input.at(1));
      dac1_cached = true;
    } catch(caribou::RegisterTypeMismatch&) {
    }
    try {
      dac2 = dev2->getRegister(input.at(5));
      dac2_cached = true;
    } catch(caribou::RegisterTypeMismatch&) {
    }

    // Sample through the DAC1 range
    for(int i = std::stoi(input.at(2)); i >= std::stoi(input.at(3)); i--) {
      LOG(INFO) << input.at(1) << " = " << i;
      dev1->setRegister(input.at(1), i);

      // Sample through the DAC2 range, trigger the PG and read back the data
      for(int j = std::stoi(input.at(6)); j >= std::stoi(input.at(7)); j--) {
        LOG(INFO) << input.at(5) << " = " << j;
        dev2->setRegister(input.at(5), j);

        std::stringstream responses;
        responses << "Pixel responses: ";
        for(int k = 0; k < std::stoi(input.at(10)); k++) {
          // Wait a bit, in ms:
          mDelay(std::stoi(input.at(9)));

          pearydata frame;
          try {
            // Send pattern:
            dev2->command("triggerPatternGenerator", "1");
            // Read the data:
            frame = dev2->getData();
          } catch(caribou::DataException& e) {
            // Retrieval failed, retry once more before aborting:
            LOG(WARNING) << e.what() << ", retyring once.";
            mDelay(10);
            frame = dev2->getData();
          }

          for(auto& px : frame) {
            myfile << i << "," << j << "," << px.first.first << "," << px.first.second << "," << (*px.second) << "\n";
          }
          responses << frame.size() << " ";
          mDelay(std::stoi(input.at(9)));
        }
        LOG(INFO) << responses.str();
      }
    }
    // Restore the old setting of the DACs:
    if(dac1_cached) {
      dev1->setRegister(input.at(1), dac1);
    }
    if(dac2_cached) {
      dev2->setRegister(input.at(4), dac2);
    }

    LOG(INFO) << "Data writte to file: \"" << filename << "\"";
  } catch(caribou::caribouException& e) {
    LOG(ERROR) << e.what();
    return ReturnCode::Error;
  }
  return ReturnCode::Ok;
}
