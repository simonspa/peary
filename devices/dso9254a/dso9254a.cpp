/**
 * Caribou Dso9254a Device implementation
 */

#include "dso9254a.hpp"
#include <string>
#include "log.hpp"

using namespace caribou;

dso9254a::dso9254a(const caribou::Configuration config) : auxiliaryDevice(config, std::string(DEFAULT_DEVICEPATH)) {
  _config = config;

  // FIXME dispatcher functions
  _dispatcher.add("query", &dso9254a::query, this);
  _dispatcher.add("send", &dso9254a::send, this);
  _dispatcher.add("waitForTrigger", &dso9254a::waitForTrigger, this);

  LOG(logDEBUG) << "New scope interface, trying to connect...";
  std::string query = "*IDN?";
  std::string data = this->receive(query).front();

  LOG(logDEBUG) << "Connected successfully to " << _devpath;
  LOG(logDEBUG) << "Scope identifier: " << data;
}

dso9254a::~dso9254a() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

void dso9254a::send(std::string command) {
  this->send(command);
}

std::string dso9254a::query(const std::string query) {
  return this->receive(query).front();
}

int dso9254a::waitForTrigger() {

  std::string query = ":ter?";

  int trigger = 0;
  while(trigger == 0) {
    mDelay(10);
    trigger = std::stoi(this->receive(query).front());
  }

  return trigger;
}

pearydata dso9254a::getData() {

  std::vector<double> values;

  LOG(logINFO) << "Retrieving data from scope...";
  std::string cmd = "wav:data?";
  std::vector<std::string> data = this->receive(cmd);

  LOG(logINFO) << "Crunching numbers...";

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

std::string dso9254a::getName() {
  return DEVICE_NAME;
}
