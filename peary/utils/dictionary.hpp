#ifndef CARIBOU_DICT_H
#define CARIBOU_DICT_H

#include <initializer_list>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "exceptions.hpp"
#include "constants.hpp"
#include "datatypes.hpp"
#include "log.hpp"

namespace caribou {

  template<typename T1, typename T2>
  class register_dict {
  public:
    register_dict() {};
    virtual ~register_dict() {};

    // Register new component alias:
    void add(const std::string name, register_t<T1, T2> reg) {
      regs.insert(std::make_pair(name,reg));
    }

    void add(const std::vector<std::pair<std::string, register_t<T1, T2> > > reg) {
      for (auto i : reg) add(i.first, i.second);
    }

    // Return register config for the name in question:
    register_t<T1, T2> get(const std::string name) const {
      try { return regs.at(name); }
      catch(...) {
	throw ConfigInvalid("Register name \"" + name + "\" unknown");
      }
    }

  private:
    /** Map fo human-readable names for periphery components
     */
    std::map<std::string,register_t<T1, T2> > regs;
  };

  class component_dict {
  public:
    component_dict() {};
    virtual ~component_dict() {};

    // Register new component alias:
    template<typename T>
    void add(const std::string name, T ptr) {
      comps.insert(std::make_pair(name,std::make_shared<T>(ptr)));
    }

    // Return shared pointer to component config for the name in question:
    template<typename T>
    std::shared_ptr<T> get(const std::string name) const {
      try {
	std::shared_ptr<component_t> ptr = comps.at(name);
	if(std::dynamic_pointer_cast<T>(ptr)) { return std::dynamic_pointer_cast<T>(ptr); }
	else { throw ConfigInvalid("Component cannot be cast"); }
      }
      catch(...) {
	throw ConfigInvalid("Component name \"" + name + "\" unknown");
      }
    }

  private:
    /** Map fo human-readable names for periphery components
     */
    std::map<std::string,std::shared_ptr<component_t>> comps;
  };

} //namespace caribou

#endif /* CARIBOU_DICT_H */
