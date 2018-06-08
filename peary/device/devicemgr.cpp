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

using namespace caribou;

caribouDeviceMgr::caribouDeviceMgr() : _deviceList() {
  LOG(logDEBUG) << "New Caribou device manager";

  if(check_flock("pearydev.lock")) {
    throw caribou::caribouException("Found running device instance, cannot start manager.");
  }

  if(!acquire_flock("pearydevmgr.lock")) {
    throw caribou::caribouException("Found running device manager instance.");
  }

  // Mark all instanciated devices as managed:
  caribou::caribouDevice::managedDevice = true;
}

caribouDeviceMgr::~caribouDeviceMgr() {
  LOG(logDEBUG) << "Deleting all Caribou devices.";

  // Call the destructor of the device instances
  for(auto it : _deviceList) {
    delete it;
  };

  // Close the loaded libraries
  for(auto it : _deviceLibraries) {
    dlclose(it.second);
  }
}

caribouDevice* caribouDeviceMgr::getDevice(size_t id) {

  if(_deviceList.size() < (id + 1)) {
    LOG(logCRITICAL) << "Device ID " << id << " not known!";
    throw caribou::DeviceException("Unknown device id");
  } else
    return _deviceList.at(id);
}

std::vector<caribouDevice*> caribouDeviceMgr::getDevices() {
  return _deviceList;
}

size_t caribouDeviceMgr::addDevice(std::string name, const caribou::Configuration config) {

  caribouDevice* deviceptr = nullptr;
  size_t device_id = 0;

  // CMake prepends "lib" to the shared library name: "lib"+LibraryName+".so"
  std::string libName = std::string("lib").append(name).append(SHARED_LIBRARY_SUFFIX);

  // Load shared library, be sure to export the path of the lib to LD_LIBRARY_PATH!
  void* hndl = nullptr;

  // Check if the library is loaded already
  auto it = _deviceLibraries.find(name);
  if(it != _deviceLibraries.end()) {
    LOG(logDEBUG) << "Shared library \"" << libName << "\" already loaded";
    hndl = (*it).second;
  } else {
    LOG(logDEBUG) << "Loading shared library \"" << libName << "\"...";
    hndl = dlopen(libName.c_str(), RTLD_NOW);
    if(!hndl) {
      LOG(logCRITICAL) << "Loading of " << libName << " failed:";
      LOG(logCRITICAL) << dlerror();
      throw caribou::DeviceLibException("dlopen could not open shared library");
    }
    _deviceLibraries.insert(std::make_pair(name, hndl));
    LOG(logDEBUG) << "Shared library successfully loaded.";
  }

  LOG(logINFO) << "Creating new instance of device \"" << name << "\".";
  try {
    // Use generator() to create the instance of the description
    LOG(logDEBUG) << "Calling device generator...";
    void* gen = dlsym(hndl, "generator");
    char* err;
    if((err = dlerror()) != NULL) {
      // handle error, the symbol wasn't found
      LOG(logCRITICAL) << err;
      throw caribou::DeviceLibException("Symbol lookup for caribouDevice* failed");
    } else {
      // symbol found, its value is in "gen"
      deviceptr = reinterpret_cast<caribouDevice* (*)(const caribou::Configuration)>(gen)(config);
    }

    device_id = _deviceList.size();
    LOG(logINFO) << "Appending instance to device list, device ID " << device_id;
    _deviceList.push_back(deviceptr);
  } catch(caribou::DeviceLibException& e) {
    LOG(logCRITICAL) << "Could not retrieve pointer to caribouDevice object";
    LOG(logCRITICAL) << "Is the generator() function included w/o C++ name mangling?";
    throw caribou::DeviceException(e.what());
  }

  return device_id;
}
