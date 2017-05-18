/* This file contains the configuration class used by the Caribou libraries
 */

#ifndef CARIBOU_CONFIG_H
#define CARIBOU_CONFIG_H

#include <iomanip>
#include <map>
#include <string>
#include <vector>

#include "utils.hpp"

namespace caribou {

  class Configuration {
  public:
    Configuration(const std::string& config = "", const std::string& section = "");
    Configuration(std::istream& conffile, const std::string& section = "");
    Configuration(const Configuration& other);
    void Save(std::ostream& file) const;
    void Load(std::istream& file, const std::string& section);
    bool SetSection(const std::string& section) const;
    bool SetSection(const std::string& section);
    std::vector<std::string> GetSections();
    std::string operator[](const std::string& key) const { return GetString(key); }
    std::string Get(const std::string& key, const std::string& def) const;
    double Get(const std::string& key, double def) const;
    int64_t Get(const std::string& key, int64_t def) const;
    uint64_t Get(const std::string& key, uint64_t def) const;
    template <typename T> T Get(const std::string& key, T def) const {
      return caribou::from_string<T>(Get(key, caribou::to_string(def)));
    }
    template <typename T> std::vector<T> Get(const std::string& key, std::vector<T> def) const {
      return split(Get(key, std::string()), def, ',');
    }
    int Get(const std::string& key, int def) const;
    template <typename T> T Get(const std::string& key, const std::string fallback, const T& def) const {
      return Get(key, Get(fallback, def));
    }
    std::string Get(const std::string& key, const char* def) const {
      std::string ret(Get(key, std::string(def)));
      return ret;
    }
    std::string Get(const std::string& key, const std::string fallback, std::string def) const {
      return Get(key, Get(fallback, def));
    }
    // std::string Get(const std::string & key, const std::string & def = "");
    template <typename T> void Set(const std::string& key, const T& val);
    std::string Name() const;
    Configuration& operator=(const Configuration& other);
    void Print(std::ostream& out) const;
    void Print() const;

  private:
    std::string GetString(const std::string& key) const;
    void SetString(const std::string& key, const std::string& val);
    typedef std::map<std::string, std::string> section_t;
    typedef std::map<std::string, section_t> map_t;
    map_t m_config;
    mutable std::string m_section;
    mutable section_t* m_cur;
  };

  inline std::ostream& operator<<(std::ostream& os, const Configuration& c) {
    c.Save(os);
    return os;
  }

  template <typename T> inline void Configuration::Set(const std::string& key, const T& val) {
    SetString(key, caribou::to_string(val));
  }

} // namespace caribou

#endif /* CARIBOU_CONFIG_H */
