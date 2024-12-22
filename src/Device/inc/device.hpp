#pragma once
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstddef>
#include <cstdint>
#include "etl/unordered_map.h"
#include "etl/vector.h"

/**
 * @file device.hpp
 * @brief This file contains the MotorBase class definition.
 */


/**
 * @defgroup Devices
 * @brief Functions related to device control.
 * @{
 */

namespace stmepic {
/**
 * @def DEVICE_MAX_DEVICE_COUNT
 * @brief Maximum number of devices that can be managed.
 */
#define DEVICE_MAX_DEVICE_COUNT (uint32_t)32


/**
 * @enum DeviceStatus
 * @brief Enumeration representing various statuses of a device.
 * 
 * @var DeviceStatus::OK
 * Device is operating normally.
 * 
 * @var DeviceStatus::DEVICE_UNKNOWN_ERROR
 * Device encountered an unknown error.
 * 
 * @var DeviceStatus::DEVICE_NOT_IMPLEMENTED
 * Device functionality is not implemented.
 * 
 * @var DeviceStatus::DEVICE_IO_ERROR
 * Device encountered an I/O error.
 * 
 * @var DeviceStatus::DEVICE_NOT_CONNECTED
 * Device is not connected.
 * 
 * @var DeviceStatus::DEVICE_POWEROFF
 * Device is powered off.
 * 
 * @var DeviceStatus::DEVICE_ALL_ERROR
 * Device encountered all possible errors.
 * 
 * @var DeviceStatus::DEVICE_TIMEOUT
 * Device operation timed out.
 * 
 * @var DeviceStatus::DEVICE_HAL_ERROR
 * Device encountered a HAL error.
 * 
 * @var DeviceStatus::DEVICE_HAL_BUSY
 * Device HAL is busy.
 */
enum class DeviceStatus{
  OK = 0,
  DEVICE_UNKNOWN_ERROR = 1,
  DEVICE_NOT_IMPLEMENTED = 2,
  DEVICE_IO_ERROR = 3,
  DEVICE_NOT_CONNECTED = 4,
  DEVICE_POWEROFF = 5,
  DEVICE_ALL_ERROR = 6,
  DEVICE_TIMEOUT = 7,
  DEVICE_HAL_ERROR = 8,
  DEVICE_HAL_BUSY = 9,
};


/**
 * @class DeviceBase
 * @brief Abstract base class for all devices.
 * 
 * This class provides the interface for device operations such as checking connection status, 
 * getting device status, resetting, starting, and stopping the device.
 */

class DeviceBase {
public:
  DeviceBase() = default;

  [[nodiscard]] virtual Result<bool> device_is_connected() = 0;

  [[nodiscard]] virtual bool device_ok() = 0;

  [[nodiscard]] virtual Result<DeviceStatus> device_get_status() = 0;

  [[nodiscard]] virtual Status device_reset() = 0;

  [[nodiscard]] virtual Status device_start() = 0;

  [[nodiscard]] virtual Status device_stop() = 0;

};


/**
 * @class DeviceTranslateStatus
 * @brief Utility class for translating HAL status to device status and general status.
 * 
 * This class provides static methods to translate HAL_StatusTypeDef to DeviceStatus and Status.
 */
class DeviceTranslateStatus {
public:
  DeviceTranslateStatus() = default;

  static DeviceStatus translate_hal_status_to_device(HAL_StatusTypeDef status) {
    switch (status) {
    case HAL_OK:
      return DeviceStatus::OK;
    case HAL_ERROR:
      return DeviceStatus::DEVICE_HAL_ERROR;
    case HAL_BUSY:
      return DeviceStatus::DEVICE_HAL_BUSY;
    case HAL_TIMEOUT:
      return DeviceStatus::DEVICE_TIMEOUT;
    default:
      return DeviceStatus::DEVICE_UNKNOWN_ERROR;
    }
  }

  static Status translate_hal_status_to_status(HAL_StatusTypeDef status) {
    switch (status) {
    case HAL_OK:
      return Status::OK();
    case HAL_ERROR:
      return Status::IOError("HAL error");
    case HAL_BUSY:
      return Status::IOError("HAL busy");
    case HAL_TIMEOUT:
      return Status::TimeOut("HAL timeout");
    default:
      return Status::UnknownError("HAL unknown error");
    }
  }
};


/**
 * @class DeviceMenager
 * @brief Template class for managing multiple devices.
 * 
 * This class provides methods to add, remove, reset, start, and stop devices. It also supports 
 * checking if all devices are connected and if all devices are operating normally.
 * 
 * @tparam MaxDeviceCount Maximum number of devices that can be managed.
 */
template <uint32_t MaxDeviceCount=DEVICE_MAX_DEVICE_COUNT>
class DeviceMenager {
public:
  using device_status_callback = void (*)(DeviceBase*,DeviceStatus);

  DeviceMenager() = default;

  Status add_device(DeviceBase *device){
    if(etl::find(devices.begin(), devices.end(), device) != devices.end()){
      return Status::AlreadyExists();
    }
    devices.push_back(device);
    return Status::OK();
  }

  Status remove_device(DeviceBase *device){
    auto dev = etl::find(devices.begin(), devices.end(), device);
    if(dev == devices.end()){
      return Status::KeyError();
    }
    (void)remove_callback(device);
    devices.erase(dev);
    return Status::OK();
  }

  Status reset_all(){
    Status status;
    for(auto &device : devices){
      status = device->device_reset();
      if(!status.ok()) return status;
    }
  };

  Status start_all(){
    Status status;;
    for(auto &device : devices){
      status = device->device_start();
      if(status.ok()) return status;
    };
  };

  Status stop_all(){
    Status status;
    for(auto &device : devices){
      status = device->device_stop();
      if(status.ok()) return status;
    };
  };

  void add_callback(DeviceBase *device,device_status_callback callback){
    add_device(device);
    device_callbacks[device] = callback;
  }

  Status remove_callback(DeviceBase *device){
    auto devcall = device_callbacks.find(device);
    if(devcall != device_callbacks.end()){
      device_callbacks.erase(devcall);
      return Status::OK();
    }
    return Status::KeyError();
  }

  [[nodiscard]] Result<bool> is_all_connected(){
    for(auto &device : devices){
      auto result = device->device_is_connected();
      if(!result.ok()) return result;
      if(!result.valueOrDie()) return Result<bool>::OK(false);
    }
    return Result<bool>::OK(true);
  }

  [[nodiscard]] bool is_all_ok(){
    for(auto &device : devices){
      if(!device->device_ok()) return false;
    }
    return true;
  }

private:  
  etl::vector<DeviceBase*,MaxDeviceCount> devices;
  etl::unordered_map<DeviceBase*,device_status_callback,MaxDeviceCount> device_callbacks;
};

} // namespace stmepic


/** @} */ // end of Encoder group