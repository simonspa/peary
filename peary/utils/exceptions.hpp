/**
 * Caribou Peary API exception classes
 */

#ifndef CARIBOU_EXCEPTIONS_H
#define CARIBOU_EXCEPTIONS_H

#include <exception>
#include <string>

namespace caribou {

  /** Base class exception to be used throughout the Caribou Peary framework.
   */
  class caribouException : public std::exception {
  public:
    caribouException(const std::string& what_arg) : std::exception(), ErrorMessage(what_arg){};
    ~caribouException() throw(){};
    virtual const char* what() const throw() { return ErrorMessage.c_str(); };

  private:
    std::string ErrorMessage;
  };

  /** Exception covering critical issues with the configuration found during runtime:
   *   - out-of-range parameters
   *   - missing (crucial) parameters
   *   - inconsistent or mismatched configuration sets
   */
  class ConfigInvalid : public caribouException {
  public:
    ConfigInvalid(const std::string& what_arg) : caribouException(what_arg) {}
  };

  /** Exception for missing but requested configuration keys
   */
  class ConfigMissingKey : public caribouException {
  public:
    ConfigMissingKey(const std::string& what_arg) : caribouException(what_arg) {}
  };

  /** Exception for missing but requested register information
   */
  class UndefinedRegister : public caribouException {
  public:
    UndefinedRegister(const std::string& what_arg) : caribouException(what_arg) {}
  };

  /** Exception for issues occuring during device setup, management and initialization
   *
   *  This comprises firmware problems as well as problems with missing device libraries.
   *  More specialized exceptions can be used, which inherit from this class (see below)
   */
  class DeviceException : public caribouException {
  public:
    DeviceException(const std::string& what_arg) : caribouException(what_arg) {}
  };

  /** Exception covering issues with loading of the peary device libraries by
   *  the device manager
   */
  class DeviceLibException : public DeviceException {
  public:
    DeviceLibException(const std::string& what_arg) : DeviceException(what_arg) {}
  };

  /** Exception covering issues with the device implementation such as missing functions
   */
  class DeviceImplException : public DeviceException {
  public:
    DeviceImplException(const std::string& what_arg) : DeviceException(what_arg) {}
  };

  /** Exception covering issues with the Caribou firmware such as missing
   *  firmware binaries, problems flashing the selected firmware or the request
   *  to configure an unsupported device
   */
  class FirmwareException : public DeviceException {
  public:
    FirmwareException(const std::string& what_arg) : DeviceException(what_arg) {}
  };

  /** Exception class covering read/write issues during communication with
   *  the configured device(s)
   */
  class CommunicationError : public DeviceException {
  public:
    CommunicationError(const std::string& what_arg) : DeviceException(what_arg) {}
  };

  /** Exception class for all Caribou exceptions related to data read from the devices
   */
  class DataException : public caribouException {
  public:
    DataException(const std::string& what_arg) : caribouException(what_arg) {}
  };

  /** Exception class thrown when requesting data in a format which is not available
   *  (e.g. for CLICpix2, asking for TOT in long-counting mode)
   */
  class WrongDataFormat : public DataException {
  public:
    WrongDataFormat(const std::string& what_arg) : DataException(what_arg) {}
  };

  /** This exception class is used in case new data are requested but nothing available. Usually
   *  this is not critical and should be caught by the caller. E.g. when runninng a DAQ with
   *  external triggering and constant event polling it can not be ensured that data
   *  are always available, but returning an empty event will mess up trigger sync.
   */
  class NoDataAvailable : public DataException {
  public:
    NoDataAvailable(const std::string& what_arg) : DataException(what_arg) {}
  };

  /** Exception inidcating an incomplete data response
   */
  class DataIncomplete : public DataException {
  public:
    DataIncomplete(const std::string& what_arg) : DataException(what_arg) {}
  };

  /** Special case of DataIncomplete: data returned from a triggered device do not contain all events
   */
  class DataMissingEvent : public DataIncomplete {
  public:
    uint32_t numberMissing;
    DataMissingEvent(const std::string& what_arg, uint32_t nmiss) : DataIncomplete(what_arg), numberMissing(nmiss) {}
  };

  /**  Exception class indicating corrupt data, i.e., the data is not decodable
   */
  class DataCorrupt : public DataException {
  public:
    DataCorrupt(const std::string& what_arg) : DataException(what_arg) {}
  };

} // namespace caribou

#endif /* CARIBOU_EXCEPTIONS_H */
