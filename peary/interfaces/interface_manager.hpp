#ifndef INTERFACE_MANAGER_HPP
#define INTERFACE_MANAGER_HPP

#include <vector>
#include <mutex>
#include <map>
#include <string>

namespace caribou {

  template <typename T>
  class interface_manager {

  protected:
    // Default constructor: protected for singleton class 
    interface_manager() {};

  private:

    // Mutex protecting interface generation
    static std::mutex mutex;

    static std::map<std::string, T&> interfaces;
    
  public:

    /* Get instance of the interface class
     *  The below function is thread-safe in C++11 and can thus
     *  be called from several HAL instances concurrently.
     *  
     *  device_path : path to the device, ex. /dev/i2c-0
     */
    static T* getInterface(std::string const & device_path) {
      static interface_manager<T> instance;
      std::lock_guard<std::mutex> lock(mutex);
      
      try{
  	return interfaces.at(device_path);
      }
      //such interface has not been opened yet
      catch(const std::out_of_range & ){
	
  	interfaces[device_path] = new T(device_path);
  	return interfaces[device_path];
      }
	
    }

    
    /* Delete unwanted functions from singleton class (C++11)
     */
    interface_manager(interface_manager const&) = delete;
    void operator=(interface_manager const&)    = delete;
  };

}

#endif /*INTERFACE_MANAGER_HPP*/
