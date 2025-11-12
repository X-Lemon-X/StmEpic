
#include "stmepic.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "ICM20948.hpp"

using namespace stmepic::sensors::imu;
using namespace stmepic::sensors::imu::internal;
using namespace stmepic;


Result<std::shared_ptr<ICM20948>> ICM20948::Make(std::shared_ptr<I2cBase> hi2c, uint8_t address, GpioPin *gpio_int) {
  if(hi2c == nullptr)
    return Status::ExecutionError("I2cBase is nullpointer");
  auto a = std::shared_ptr<ICM20948>(new ICM20948(hi2c, address, gpio_int));
  return Result<std::shared_ptr<ICM20948>>::OK(std::move(a));
}

ICM20948::ICM20948(std::shared_ptr<I2cBase> hi2c, uint8_t _address, GpioPin *_gpio_int)

: hi2c(hi2c), _device_status(Status::Disconnected("not started")), reading_status(Status::OK()),
  address(_address), gpio_int(_gpio_int) {
}

Status ICM20948::device_get_status() {
  return _device_status;
}


Status ICM20948::stop() {
  // uint8_t data = BMP280_RESET_VALUE;
  // return hi2c->write(address, BMP280_REG_RESET, &data, 1);
  return Status::OK();
}

Status ICM20948::init() {
  STMEPIC_ASSING_TO_OR_RETURN(_device_status, hi2c->is_device_ready(address, 1, 500));
  uint8_t data[2] = {};
  data[0]         = ICM20948_PAGE_0;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, ICM20948_REG_PAGE, data, 1));
  data[0] = 0;
  STMEPIC_RETURN_ON_ERROR(hi2c->read(address, ICM20948_REG_WHO_AM_I, data, 1));
  if(data[0] != ICM20948_WHO_AM_I) {
    _device_status = Status::Disconnected("ICM20948 is not recognized");
    return _device_status;
  }


  return Status::OK();
}

Status ICM20948::do_device_task_reset() {
  // STMEPIC_RETURN_ON_ERROR(device_stop());
  // return device_start();
  return Status::OK();
}

Status ICM20948::device_set_settings(const DeviceSettings &settings) {
  (void)settings;
  return Status::OK();
}

Status ICM20948::do_device_task_start() {
  return DeviceThreadedBase::do_default_task_start(task_bar, task_bar_before, this);
}

Status ICM20948::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}

bool ICM20948::device_ok() {
  return _device_status.ok();
}

Result<bool> ICM20948::device_is_connected() {
  return Result<bool>::Propagate(_device_status.ok(), std::move(_device_status));
}


Status ICM20948::task_bar_before(SimpleTask &handler, void *arg) {
  (void)handler;
  ICM20948 *bar = static_cast<ICM20948 *>(arg);
  return bar->init();
}

Status ICM20948::task_bar(SimpleTask &handler, void *arg) {
  (void)handler;
  ICM20948 *imu = static_cast<ICM20948 *>(arg);
  imu->handle();
  return Status::OK();
}

Status ICM20948::handle() {
  auto maybe_data = read_data();
  _device_status  = maybe_data.status();
  if(maybe_data.ok()) {
    bar_data = maybe_data.valueOrDie();
  }
  return _device_status;
}

Result<ImuData> ICM20948::read_data() {
  uint8_t regs[6] = {};
  ImuData data    = {};
  // TOODO

  return Result<ImuData>::OK(std::move(data));
}

Result<ImuData> ICM20948::get_data() {
  return Result<ImuData>::Propagate(std::move(bar_data), std::move(_device_status));
}
