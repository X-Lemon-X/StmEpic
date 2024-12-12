#pragma once
#include "stmepic.hpp"
#include "stmepic_status.hpp"


namespace stmepic {

enum class DeviceStatus{
  OK = 0,
  DEVICE_NOT_CONNECTED = 4,
  DEVICE_UNKNOWN_ERROR = 1,
  DEVICE_NOT_IMPLEMENTED = 2,
  DEVICE_IO_ERROR = 3,
  DEVICE_POWEROFF = 5,
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


} // namespace stmepic