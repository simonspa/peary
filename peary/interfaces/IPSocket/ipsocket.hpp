#ifndef CARIBOU_HAL_IPSOCKET_HPP
#define CARIBOU_HAL_IPSOCKET_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  typedef uint32_t ipsocket_port_t;
  typedef std::string ipsocket_t;

  class iface_ipsocket : public Interface<ipsocket_port_t, ipsocket_t, ipsocket_t> {

  private:
    // Default constructor: private (only created by InterfaceManager)
    //
    // It can throw DeviceException
    iface_ipsocket(std::string const& ip_address);

    virtual ~iface_ipsocket();

    // Protects access to the bus
    std::mutex mutex;

    template <typename T> friend class caribouHAL;

  public:
    ipsocket_t write(const ipsocket_port_t& address, const ipsocket_t& payload);

    std::vector<ipsocket_t> read(const ipsocket_port_t& address, const ipsocket_t& query, const unsigned int length = 1);

  private:
    // Unused constructor
    iface_ipsocket() = delete;

    // Split the devicePath into port and IP address:
    std::pair<std::string, uint32_t> split_ip_address(std::string address);

    std::string trim(const std::string& str, const std::string& delims = " \t\n\r\v");
    std::string cleanCommandString(std::string& command);

    // Remote socket to connect to
    int mysocket_;

    // only this function can create the interface
    friend iface_ipsocket& InterfaceManager::getInterface<iface_ipsocket>(std::string const&);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_IPSOCKET_HPP */
