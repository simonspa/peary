/**
 * @file
 * @brief Special file automatically included in the device for the dynamic loading
 *
 * Needs the following names to be defined by the build system
 * - DEVICE_NAME: name of the device
 * - DEVICE_HEADER: name of the header defining the device
 */

#ifndef PEARY_DEVICE_NAME
#error "This header should only be automatically included during the build"
#endif

#include <memory>
#include <utility>

#include "configuration.hpp"
#include "device.hpp"

#include PEARY_DEVICE_HEADER

namespace caribou {
  extern "C" {
  /**
   * @brief Instantiates a peary device
   * @param config Configuration for this module
   * @param messenger Pointer to the Messenger (guarenteed to be valid until the module is destructed)
   * @param detector Pointer to the Detector object this module is bound to
   * @return Instantiation of the module
   *
   * Internal method for the dynamic loading in the ModuleManager. Forwards the supplied arguments to the constructor and
   * returns an instantiation
   */
  caribouDevice* generator(const Configuration config);
  caribouDevice* generator(const Configuration config) {
    auto device = new PEARY_DEVICE_NAME(config); // NOLINT
    return static_cast<caribouDevice*>(device);
  }
  }
} // namespace caribou
