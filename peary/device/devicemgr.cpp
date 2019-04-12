/**
 * @file
 * @brief Caribou Device Manager class implementation
 */

#include "devicemgr.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "utils.hpp"

#include <algorithm>
#include <dlfcn.h>
#include <string>
#include <vector>

// Common prefix for all devices
// TODO [doc] Should be provided by the build system
#define PEARY_DEVICE_PREFIX "libPearyDevice"

// These should point to the function defined in dynamic_module_impl.cpp
#define PEARY_GENERATOR_FUNCTION "peary_device_generator"

using namespace caribou;

DeviceMgr::DeviceMgr() : _deviceList() {
  LOG(DEBUG) << "New Caribou device manager";

  if(check_flock("pearydev.lock")) {
    throw caribou::caribouException("Found running device instance, cannot start manager.");
  }

  if(!acquire_flock("pearydevmgr.lock")) {
    throw caribou::caribouException("Found running device manager instance.");
  }

  // Mark all instanciated devices as managed:
  caribou::Device::managedDevice = true;
}

DeviceMgr::~DeviceMgr() {
  LOG(DEBUG) << "Deleting all Caribou devices.";

  clearDevices();

  // Close the loaded libraries
  for(auto it : _deviceLibraries) {
    dlclose(it.second);
  }
}

Device* DeviceMgr::getDevice(size_t id) {

  if(_deviceList.size() < (id + 1)) {
    throw caribou::DeviceException("Device ID " + std::to_string(id) + " not known!");
  } else
    return _deviceList.at(id);
}

std::vector<Device*> DeviceMgr::getDevices() {
  return _deviceList;
}

size_t DeviceMgr::addDevice(std::string name, const caribou::Configuration config) {

  Device* deviceptr = nullptr;
  size_t device_id = 0;

  // Load library for each device. Libraries are named (by convention + CMAKE) libPearyDevice Name.suffix
  std::string libName = std::string(PEARY_DEVICE_PREFIX).append(name).append(SHARED_LIBRARY_SUFFIX);

  // Load shared library, be sure to export the path of the lib to LD_LIBRARY_PATH!
  void* hndl = nullptr;

  // Check if the library is loaded already
  auto it = _deviceLibraries.find(name);
  if(it != _deviceLibraries.end()) {
    LOG(DEBUG) << "Shared library \"" << libName << "\" already loaded";
    hndl = (*it).second;
  } else {
    LOG(DEBUG) << "Loading shared library \"" << libName << "\"...";
    hndl = dlopen(libName.c_str(), RTLD_NOW);
    if(!hndl) {
      LOG(FATAL) << "Loading of " << libName << " failed:";
      LOG(FATAL) << dlerror();
      throw caribou::DeviceLibException("dlopen could not open shared library");
    }
    _deviceLibraries.insert(std::make_pair(name, hndl));
    LOG(DEBUG) << "Shared library successfully loaded.";
  }

  LOG(INFO) << "Creating new instance of device \"" << name << "\".";
  try {
    // Use generator() to create the instance of the description
    LOG(DEBUG) << "Calling device generator...";
    void* gen = dlsym(hndl, PEARY_GENERATOR_FUNCTION);
    char* err;
    if((err = dlerror()) != NULL) {
      // handle error, the symbol wasn't found
      LOG(FATAL) << err;
      throw caribou::DeviceLibException("Symbol lookup for Device* failed");
    } else {
      // symbol found, its value is in "gen"
      deviceptr = reinterpret_cast<Device* (*)(const caribou::Configuration)>(gen)(config);
    }

    device_id = _deviceList.size();
    LOG(INFO) << "Appending instance to device list, device ID " << device_id;
    _deviceList.push_back(deviceptr);
  } catch(caribou::DeviceLibException& e) {
    LOG(FATAL) << "Could not retrieve pointer to Device object";
    LOG(FATAL) << "Is the generator() function included w/o C++ name mangling?";
    throw caribou::DeviceException(e.what());
  }

  return device_id;
}

void DeviceMgr::clearDevices() {
  // Call the destructor of the device instances
  for(auto it : _deviceList) {
    delete it;
  };

  // Clear the list:
  _deviceList.clear();
}
