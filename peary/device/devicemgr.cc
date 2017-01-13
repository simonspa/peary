/**
 * Caribou Device Manager class implementation
 */

#include "devicemgr.h"
#include "device.h"
#include "log.h"
#include "utils.h"
#include "exceptions.h"

#include <string>
#include <vector>
#include <dlfcn.h>
#include <algorithm>

#ifdef __APPLE__
#define SHARED_LIB ".dylib"
#else
#define SHARED_LIB ".so"
#endif

using namespace caribou;

caribouDeviceMgr::caribouDeviceMgr() :
  _deviceList() {
  LOG(logDEBUG) << "New Caribou device manager";
}

caribouDeviceMgr::~caribouDeviceMgr() {
  LOG(logDEBUG) << "Deleting all Caribou devices.";

  // Call the destructor of the device instances
  for(auto it : _deviceList) { delete it; };

  // Close the loaded libraries
  for(auto it : _deviceLibraries) { dlclose(it.second); }
};

caribouDevice* caribouDeviceMgr::getDevice(size_t id) {

  if(_deviceList.size() < (id+1)) {
    LOG(logCRITICAL) << "Device ID " << id << " not known!";
    throw caribou::DeviceException("Unknown device id");
  }
  else return _deviceList.at(id);
}

size_t caribouDeviceMgr::addDevice(std::string name, const caribou::Configuration config) {

  caribouDevice* deviceptr = nullptr;
  size_t device_id = 0;
  
  // CMake prepends "lib" to the shared library name: "lib"+LibraryName+".so"
  std::string libName = std:: string("lib").append(name).append(SHARED_LIB);
		
  // Load shared library, be sure to export the path of the lib to LD_LIBRARY_PATH!
  void *hndl = nullptr;

  // Check if the library is loaded already
  auto it = _deviceLibraries.find(name);
  if(it != _deviceLibraries.end()) {
    LOG(logDEBUG) << "Shared library \"" << libName << "\" already loaded";
    hndl = (*it).second;
  }
  else {
    LOG(logDEBUG) << "Loading shared library \"" << libName << "\"...";
    hndl = dlopen(libName.c_str(), RTLD_NOW);
    if(!hndl) {
      LOG(logCRITICAL) << "Loading of " << libName << " failed:";
      LOG(logCRITICAL) << dlerror();
      throw caribou::DeviceException("dlopen could not open shared library");
    }
    _deviceLibraries.insert( std::make_pair(name, hndl) );
    LOG(logDEBUG) << "Shared library successfully loaded.";
  }
  
  LOG(logINFO) << "Creating new instance of device \"" << name << "\".";
  try {
    // Use generator() to create the instance of the description
    LOG(logDEBUG) << "Calling device generator...";
    void *gen = dlsym(hndl, "generator");
    char *err;
    if ((err = dlerror()) != NULL) {
      // handle error, the symbol wasn't found
      LOG(logCRITICAL) << err;
      throw caribou::DeviceException("Symbol lookup failed");
    } else {
      // symbol found, its value is in "gen"
      deviceptr = reinterpret_cast<caribouDevice*(*)(const caribou::Configuration)>(gen)(config);
    }

    device_id = _deviceList.size();
    LOG(logINFO) << "Appending instance to device list, device ID " << device_id;
    _deviceList.push_back(deviceptr);
  }
  catch(...) {
    LOG(logCRITICAL) << "Could not retrieve pointer to caribouDevice object, is the generator() function included w/o C++ name mangling?";
    throw caribou::DeviceException("Retrieval of caribouDevice* failed");
  }

  // FIXME initialize device here? requires configuration!
  return device_id;
}
