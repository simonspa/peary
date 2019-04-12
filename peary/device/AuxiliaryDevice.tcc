/**
 * Caribou Auxiliary implementation
 */

#ifndef CARIBOU_DEVICE_AUXILIARY_IMPL
#define CARIBOU_DEVICE_AUXILIARY_IMPL

namespace caribou {

  template <typename T>

  AuxiliaryDevice<T>::AuxiliaryDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr)
      : Device(config), _devpath(devpath), _devaddress(devaddr), _config(config) {

    _devpath = config.Get<std::string>("devicepath", devpath);
    T& dev_iface = interface_manager::getInterface<T>(_devpath);
    LOG(DEBUG) << "Auxiliary device initialized at " << dev_iface.devicePath();
  }

  template <typename T> AuxiliaryDevice<T>::~AuxiliaryDevice() {}

  template <typename T> std::string AuxiliaryDevice<T>::getType() { return PEARY_DEVICE_NAME; }

  template <typename T> typename T::data_type AuxiliaryDevice<T>::send(const typename T::data_type& data) {
    return interface_manager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::vector<typename T::data_type> AuxiliaryDevice<T>::send(const std::vector<typename T::data_type>& data) {
    return interface_manager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::pair<typename T::reg_type, typename T::data_type>
  AuxiliaryDevice<T>::send(const std::pair<typename T::reg_type, typename T::data_type>& data) {
    return interface_manager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::vector<typename T::data_type> AuxiliaryDevice<T>::send(const typename T::reg_type& reg,
                                                              const std::vector<typename T::data_type>& data) {
    return interface_manager::getInterface<T>(_devpath).write(_devaddress, reg, data);
  }

  template <typename T>
  std::vector<std::pair<typename T::reg_type, typename T::data_type>>
  AuxiliaryDevice<T>::send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data) {
    return interface_manager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T> std::vector<typename T::data_type> AuxiliaryDevice<T>::receive(const unsigned int length) {
    return interface_manager::getInterface<T>(_devpath).read(_devaddress, length);
  }

  template <typename T>
  std::vector<typename T::data_type> AuxiliaryDevice<T>::receive(const typename T::reg_type reg, const unsigned int length) {
    return interface_manager::getInterface<T>(_devpath).read(_devaddress, reg, length);
  }

} // namespace caribou

#endif /* CARIBOU_DEVICE_AUXILIARY_IMPL */
