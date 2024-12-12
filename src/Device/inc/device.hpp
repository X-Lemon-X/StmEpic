#pragma once
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstddef>
#include <cstdint>
#include "etl/unordered_map.h"
#include "etl/vector.h"


namespace stmepic {
#define DEVICE_MAX_DEVICE_COUNT (uint32_t)32

enum class DeviceStatus{
  OK = 0,
  DEVICE_UNKNOWN_ERROR = 1,
  DEVICE_NOT_IMPLEMENTED = 2,
  DEVICE_IO_ERROR = 3,
  DEVICE_NOT_CONNECTED = 4,
  DEVICE_POWEROFF = 5,
  DEVICE_ALL_ERROR = 6
};

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