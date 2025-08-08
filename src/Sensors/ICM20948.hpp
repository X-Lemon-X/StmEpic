#pragma once

#include "device.hpp"
#include "gpio.hpp"
#include "stmepic.hpp"
#include "vectors3d.hpp"
#include "i2c.hpp"
#include <memory>

using namespace stmepic;
using namespace stmepic::algorithm;

/**
 * @file ICM20948.hpp
 * @brief  ICM20948  imu sensor class definition.
 */


/**
 * @defgroup Sensors
 * @{
 */

/**
 * @defgroup imu_sensors IMU
 * @{
 */

/**
 * @defgroup ICM20948_imu_sensors ICM20948
 * @brief ICM20948 IMU sensors.
 * @{
 */


namespace stmepic::sensors::imu::internal {

static const uint8_t ICM20948_I2C_ADDRESS_1    = 0x68; // first address of the BMP280 device
static const uint8_t ICM20948_I2C_ADDRESS_2    = 0x69; // second address of the BMP280 device
static const uint8_t ICM20948_REG_PAGE         = 0x7F; // select page reg
static const uint8_t ICM20948_PAGE_0           = 0x00;
static const uint8_t ICM20948_PAGE_1           = 0x10;
static const uint8_t ICM20948_PAGE_2           = 0x20;
static const uint8_t ICM20948_PAGE_3           = 0x30;
static const uint8_t ICM20948_REG_WHO_AM_I     = 0x00; // beginning reg for who am i
static const uint8_t ICM20948_WHO_AM_I         = 0xEA; // ICM20948 chip id
static const uint8_t ICM20948_REG_ACCEL_XOUT_H = 0x2D; // beginning reg for acceleration
static const uint8_t ICM20948_REG_ACCEL_LENGTH = 6;    // length of acceleration data
static const uint8_t ICM20948_REG_GYRO_XOUT_H  = 0x33; // beginning reg for gyroscope
static const uint8_t ICM20948_REG_GYRO_LENGTH  = 6;    // length of gyroscope data
static const uint8_t ICM20948_REG_TEMP_OUT_H   = 0x39; // beginning reg for temperature
static const uint8_t ICM20948_REG_TEMP_LENGTH  = 2;    // length of temperature data

} // namespace stmepic::sensors::imu::internal

namespace stmepic::sensors::imu {

/// @brief ICM20948 IMU sensor data structure
struct ICM20948_Data_t {
  float acceleration_x;
  float acceleration_y;
  float acceleration_z;

  float gyration_x;
  float gyration_y;
  float gyration_z;

  float magnetic_x;
  float magnetic_y;
  float magnetic_z;

  float temperature;
};

/**
 * @brief BMP280 barometer
 * BMP280 is a barometr and temperature sensor
 *
 */
class ICM20948 : public stmepic::DeviceThreadedBase {
public:
  /**
   * @brief Make new ICM20948 imu sensor
   *
   * @param hi2c the I2C handle that will be used to communicate with the ICM20948 device
   * @param address the address of the ICM20948 device one of two possible addresses
   * @return Brand new ICM20948 object
   */
  static Result<std::shared_ptr<ICM20948>>
  Make(std::shared_ptr<I2C> hi2c, uint8_t address = internal::ICM20948_I2C_ADDRESS_1, GpioPin *gpio_int = nullptr);

  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;
  Status device_set_settings(const DeviceSettings &settings) override;

  /**
   * @brief Get last read data from the ICM20948 sensor
   * @return ICM20948_Data_t data from the ICM20948 sensor
   */
  Result<ICM20948_Data_t> get_data();


private:
  ICM20948(std::shared_ptr<I2C> hi2c, uint8_t address, GpioPin *gpio_int = nullptr);

  stmepic::Status do_device_task_start() override;
  stmepic::Status do_device_task_stop() override;
  Status do_device_task_reset() override;

  Status init();
  Status stop();

  Result<ICM20948_Data_t> read_data();

  static Status task_bar_before(SimpleTask &handler, void *arg);
  static Status task_bar(SimpleTask &handler, void *arg);
  Status handle();

  ICM20948_Data_t bar_data;
  std::shared_ptr<I2C> hi2c;
  Status _device_status;
  Status reading_status;
  uint8_t address;
  GpioPin *gpio_int;
};

} // namespace stmepic::sensors::imu