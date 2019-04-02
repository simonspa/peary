#ifndef CARIBOU_DICT_H
#define CARIBOU_DICT_H

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "constants.hpp"
#include "datatypes.hpp"
#include "exceptions.hpp"
#include "log.hpp"

namespace caribou {

  template <typename T1, typename T2> class register_dict {
  public:
    register_dict(){};
    virtual ~register_dict(){};

    // Register new component alias:
    void add(std::string name, const register_t<T1, T2> reg) {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      regs.insert(std::make_pair(name, reg));
    }

    void add(const std::vector<std::pair<std::string, register_t<T1, T2>>> reg) {
      for(auto i : reg)
        add(i.first, i.second);
    }

    // Return register config for the name in question:
    register_t<T1, T2> get(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      try {
        return regs.at(name);
      } catch(...) {
        throw ConfigInvalid("Register name \"" + name + "\" unknown");
      }
    }

    // Check if register entry exists
    bool has(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return !(regs.find(name) == regs.end());
    }

    // Return all register names:
    std::vector<std::string> getNames() const {
      std::vector<std::string> names;
      for(auto reg : regs) {
        names.push_back(reg.first);
      }
      return names;
    }

  private:
    /** Map fo human-readable names for periphery components
     */
    std::map<std::string, register_t<T1, T2>> regs;
  };

  class component_dict {
  public:
    component_dict(){};
    virtual ~component_dict(){};

    // Register new component alias:
    template <typename T> void add(std::string name, T ptr) {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      comps.insert(std::make_pair(name, std::make_shared<T>(ptr)));
    }

    // Return shared pointer to component config for the name in question:
    template <typename T> std::shared_ptr<T> get(std::string name) const {
      try {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::shared_ptr<component_t> ptr = comps.at(name);
        if(std::dynamic_pointer_cast<T>(ptr)) {
          return std::dynamic_pointer_cast<T>(ptr);
        } else {
          throw ConfigInvalid("Component cannot be cast");
        }
      } catch(...) {
        throw ConfigInvalid("Component name \"" + name + "\" unknown");
      }
    }

    // Check if register entry exists
    bool has(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return !(comps.find(name) == comps.end());
    }

  private:
    /** Map fo human-readable names for periphery components
     */
    std::map<std::string, std::shared_ptr<component_t>> comps;
  };

  class memory_dict {
  public:
    memory_dict(){};
    virtual ~memory_dict(){};

    // Register new memory map alias:
    void add(std::string name, const memory_map mem_map) {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      mem.insert(std::make_pair(name, mem_map));
    }

    void add(const std::vector<std::pair<std::string, memory_map>> mem_map) {
      for(auto i : mem_map)
        add(i.first, i.second);
    }

    // Return memory for the name in question:
    memory_map get(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      try {
        return mem.at(name);
      } catch(...) {
        throw ConfigInvalid("Memory map name \"" + name + "\" unknown");
      }
    }

    // Check if memory entry exists
    bool has(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return !(mem.find(name) == mem.end());
    }

    // Return all memory map names:
    std::vector<std::string> getNames() const {
      std::vector<std::string> names;
      for(auto reg : mem) {
        names.push_back(reg.first);
      }
      return names;
    }

  private:
    /** Map fo human-readable names for memory pages
     */
    std::map<std::string, memory_map> mem;
  };

} // namespace caribou

#endif /* CARIBOU_DICT_H */
