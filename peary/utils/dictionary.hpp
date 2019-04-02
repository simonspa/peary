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

  /**
  * @brief Dictionary class for storing elements for lookup
  */
  template <class T> class dictionary {
  public:
    dictionary(const std::string& title) : _title(title){};
    virtual ~dictionary(){};

    // Register new element:
    template <class C> void add(std::string name, const C elem) {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      auto ptr = std::make_shared<C>(elem);
      try {
        _elements.insert(std::make_pair(name, std::dynamic_pointer_cast<T>(ptr)));
      } catch(...) {
        throw ConfigInvalid("Cannot insert " + _title + " with name \"" + name + "\" into dictionary");
      }
    }

    void add(const std::vector<std::pair<std::string, T>> elements) {
      for(auto i : elements)
        add(i.first, i.second);
    }

    // Return register config for the name in question:
    T get(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      try {
        return (*_elements.at(name));
      } catch(...) {
        throw ConfigInvalid(_title + " name \"" + name + "\" unknown");
      }
    }

    // Return shared pointer to component config for the name in question:
    template <typename C> std::shared_ptr<C> get(std::string name) const {
      try {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::shared_ptr<T> ptr = _elements.at(name);
        if(std::dynamic_pointer_cast<C>(ptr)) {
          return std::dynamic_pointer_cast<C>(ptr);
        } else {
          throw ConfigInvalid(_title + " cannot be cast");
        }
      } catch(...) {
        throw ConfigInvalid(_title + " name \"" + name + "\" unknown");
      }
    }

    // Check if register entry exists
    bool has(std::string name) const {
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return !(_elements.find(name) == _elements.end());
    }

    // Return all register names:
    std::vector<std::string> getNames() const {
      std::vector<std::string> names;
      for(auto element : _elements) {
        names.push_back(element.first);
      }
      return names;
    }

  private:
    /** Map fo human-readable names for periphery components
    */
    std::map<std::string, std::shared_ptr<T>> _elements;
    std::string _title;
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
