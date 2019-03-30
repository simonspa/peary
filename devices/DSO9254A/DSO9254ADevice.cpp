/**
 * Caribou Dso9254a Device implementation
 */

#include "DSO9254ADevice.hpp"
#include <string>
#include "log.hpp"

using namespace caribou;

DSO9254ADevice::DSO9254ADevice(const caribou::Configuration config)
    : auxiliaryDevice(config, std::string(DEFAULT_DEVICEPATH)) {
  _config = config;

  // FIXME dispatcher functions
  _dispatcher.add("query", &DSO9254ADevice::query, this);
  _dispatcher.add("send", &DSO9254ADevice::send, this);
  _dispatcher.add("waitForTrigger", &DSO9254ADevice::waitForTrigger, this);

  LOG(DEBUG) << "New scope interface, trying to connect...";
  std::string query = "*IDN?";
  std::string data = auxiliaryDevice<iface_ipsocket>::receive(query).front();

  LOG(DEBUG) << "Connected successfully to " << _devpath;
  LOG(DEBUG) << "Scope identifier: " << data;
}

DSO9254ADevice::~DSO9254ADevice() {
  LOG(INFO) << "Shutdown, delete device.";
}

void DSO9254ADevice::send(std::string command) {
  auxiliaryDevice<iface_ipsocket>::send(command);
}

std::string DSO9254ADevice::query(const std::string query) {
  return auxiliaryDevice<iface_ipsocket>::receive(query).front();
}

int DSO9254ADevice::waitForTrigger() {

  std::string query = ":ter?";

  int trigger = 0;
  while(trigger == 0) {
    mDelay(10);
    trigger = std::stoi(auxiliaryDevice<iface_ipsocket>::receive(query).front());
  }

  return trigger;
}

pearydata DSO9254ADevice::getData() {

  std::vector<double> values;

  LOG(INFO) << "Retrieving data from scope...";
  std::string cmd = "wav:data?";
  std::vector<std::string> data = auxiliaryDevice<iface_ipsocket>::receive(cmd);

  LOG(INFO) << "Crunching numbers...";

  if(data.empty()) {
    return pearydata();
    // return std::vector<double>();
  }

  std::istringstream ss(data.front());
  std::string token;

  while(std::getline(ss, token, ',')) {
    values.push_back(std::stod(token));
  }
  // return values;
  return pearydata();
}
