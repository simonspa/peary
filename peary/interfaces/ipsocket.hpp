#ifndef CARIBOU_HAL_IPSOCKET_HPP
#define CARIBOU_HAL_IPSOCKET_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "interface.hpp"
#include "interface_manager.hpp"

namespace caribou {

  typedef uint32_t ipsocket_port_t;
  typedef std::string ipsocket_t;

  class iface_ipsocket : public Interface<ipsocket_port_t, ipsocket_t, ipsocket_t> {

  private:
    // Default constructor: private (only created by interface_manager)
    //
    // It can throw DeviceException
    iface_ipsocket(std::string const& ip_address);

    virtual ~iface_ipsocket();

    // Protects access to the bus
    std::mutex mutex;

    template <typename T> friend class caribouHAL;

  public:
    ipsocket_t write(const ipsocket_port_t& address, ipsocket_t& data);

    std::vector<ipsocket_t> read(const ipsocket_port_t& address, ipsocket_t& query, const unsigned int length = 1);

    // Unused constructor
    iface_ipsocket() = delete;

    // Split the devicePath into port and IP address:
    std::pair<std::string, uint32_t> split_ip_address(std::string address);

    // Remote socket to connect to
    int mysocket_;

    // only this function can create the interface
    friend iface_ipsocket& interface_manager::getInterface<iface_ipsocket>(std::string const&);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_IPSOCKET_HPP */
