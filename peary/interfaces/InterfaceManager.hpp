#ifndef INTERFACE_MANAGER_HPP
#define INTERFACE_MANAGER_HPP

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "interface.hpp"

namespace caribou {

  class InterfaceManager {

  protected:
    // Default constructor: protected for singleton class
    InterfaceManager(){};

  private:
    // Static instance of this singleton class
    static InterfaceManager instance;

    // Mutex protecting interface generation
    static std::mutex mutex;

  public:
    /* Get instance of the interface class
     *
     *  device_path : path to the device, ex. /dev/i2c-0
     */
    template <typename T> static T& getInterface(std::string const& device_path) {
      static std::map<std::string, T*> interfaces;

      std::lock_guard<std::mutex> lock(mutex);

      try {
        return *interfaces.at(device_path);
      }
      // such interface has not been opened yet
      catch(const std::out_of_range&) {

        interfaces[device_path] = new T(device_path);
        return *interfaces[device_path];
      }
    }

    /* Delete unwanted functions from singleton class (C++11)
     */
    InterfaceManager(InterfaceManager const&) = delete;
    void operator=(InterfaceManager const&) = delete;
  };

} // namespace caribou

#endif /*INTERFACE_MANAGER_HPP*/
