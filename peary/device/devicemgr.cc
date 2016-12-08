/**
 * Caribou Device Manager class implementation
 */

#include "devicemgr.h"
#include "device.h"
#include "log.h"

#include <string>
#include <vector>
#include <dlfcn.h>

using namespace caribou;

caribouDeviceMgr::caribouDeviceMgr() :
  _deviceList() {
  LOG(logDEBUG) << "New Caribou device manager";
}

caribouDeviceMgr::~caribouDeviceMgr() {};

caribouDevice* caribouDeviceMgr::getDevice(size_t id) {

  if(_deviceList.size() < (id+1)) {
    LOG(logCRITICAL) << "Device ID " << id << " not known!";
    throw std::runtime_error("Unknown device id");
  }
  else return _deviceList.at(id);
}

size_t caribouDeviceMgr::addDevice(std::string name) {

  caribouDevice* deviceptr = nullptr;
  size_t device_id = 0;
  
  LOG(logINFO) << "Creating new instance of device \"" << name << "\".";
		
  // CMake prepends "lib" to the shared library name: "lib"+LibraryName.so
  std::string libName = std:: string("lib").append(name);
		
  // Load shared library, be sure to export the path of the lib to LD_LIBRARY_PATH!
  void *hndl = dlopen(libName.c_str(), RTLD_NOW);
  if(hndl == NULL) {
    LOG(logCRITICAL) << "Loading of " << libName << " failed:";
    LOG(logCRITICAL) << dlerror();
    throw std::runtime_error("dlopen could not open shared library");
  }
  LOG(logDEBUG) << "Shared library successfully loaded.";
  
  try {
    // Use generator() to create the instance of the description
    LOG(logDEBUG) << "Calling device generator...";
    void *gen = dlsym(hndl, "generator");
    char *err;
    if ((err = dlerror()) != NULL) {
      // handle error, the symbol wasn't found
      LOG(logCRITICAL) << err;
      throw std::runtime_error("Symbol lookup failed");
    } else {
      // symbol found, its value is in "gen"
      deviceptr = reinterpret_cast<caribouDevice*(*)()>(gen)();
    }

    device_id = _deviceList.size();
    LOG(logINFO) << "Appending instance to device list, device ID " << device_id;
    _deviceList.push_back(deviceptr);
  }
  catch(...) {
    LOG(logCRITICAL) << "Could not retrieve pointer to caribouDevice object, is the generator() function included w/o C++ name mangling?";
    throw std::runtime_error("Retrieval of caribouDevice* failed");
  }

  // FIXME initialize device here? requires configuration!
  return device_id;
}
