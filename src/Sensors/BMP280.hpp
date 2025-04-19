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
 * @file BMP280.hpp
 * @brief  BMP280  barometer sensor class definition.
 */

/**
 * @defgroup Barometer_Sensors barometer sensors
 * @brief Functions related to barometer sensors.
 * @{
 */


namespace stmepic::sensors::barometer::internal {

static const uint8_t BMP280_I2C_ADDRESS_1 = 0x76; // first address of the BMP280 device
static const uint8_t BMP280_I2C_ADDRESS_2 = 0x77; // second address of the BMP280 device
static const uint8_t BMP280_REG_DIG_T1    = 0x88; // beginning reg for dig_T1
static const uint8_t BMP280_REG_CHIP_ID   = 0xD0; // beginning reg for dig_T2
static const uint8_t BMP280_CHIP_ID       = 0x58; // BMP280 chip id
static const uint8_t BMP280_REG_PRES_MSB  = 0xF7; // beginning reg for pressure
static const uint8_t BMP280_REG_TEMO_MSB  = 0xFA; // beginning reg for temperature
static const uint8_t BMP280_REG_RESET     = 0xE0;
static const uint8_t BMP280_RESET_VALUE   = 0xB6; // reset value
static const uint8_t BMP280_REG_CTRL_MEAS = 0xF4; // register for data acquisition options of the device.
static const uint8_t BMP280_REG_CONFIG    = 0xF5; // config register

} // namespace stmepic::sensors::barometer::internal

namespace stmepic::sensors::barometer {

struct BMP280_Data_t // BMP280 data structure
{
  float temp;
  float pressure;
};

/**
 * @brief BMP280 barometer
 * BMP280 is a barometr and temperature sensor
 *
 */
class BMP280 : public stmepic::DeviceThreadedBase {
public:
  /**
   * @brief Make new BMP280 barometer sensor
   *
   * @param hi2c the I2C handle that will be used to communicate with the BMP280 device
   * @param address the address of the BMP280 device one of two possible addresses
   * @return Brand new BMP280 object
   */
  static Result<std::shared_ptr<BMP280>> Make(std::shared_ptr<I2C> hi2c, uint8_t address = internal::BMP280_I2C_ADDRESS_1);

  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;
  Status device_reset() override;
  Status device_start() override;
  Status device_stop() override;

  /**
   * @brief Get last read data from the BMP280 sensor
   * @return BMP280_Data_t data from the BMP280 sensor
   */
  Result<BMP280_Data_t> get_data();


private:
  BMP280(std::shared_ptr<I2C> hi2c, uint8_t address);

  stmepic::Status do_device_task_start() override;
  stmepic::Status do_device_task_stop() override;
  Result<BMP280_Data_t> read_data();

  static void task_bar_before(SimpleTask &handler, void *arg);
  static void task_bar(SimpleTask &handler, void *arg);
  void handle();

  /**
   * @brief Fucnito to convert the raw temperature data to float
   *
   * @param adc_T raw temperature data
   * @return temperature in Celsius
   */
  float bmp280_compensate_T_int32(int32_t adc_T);

  /**
   * @brief Fucnito to convert the raw pressure data to float in 64 bit
   *
   * @param adc_P raw pressure data
   * @return pressure in hPa
   */
  float bmp280_compensate_P_int64(int32_t adc_P);

  /**
   * @brief Fucnito to convert the raw pressure data to float in 32 bit
   *
   * @param adc_P raw pressure data
   * @return pressure in hPa
   */
  float bmp280_compensate_P_int32(int32_t adc_P);

  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;

  int32_t t_fine;


  BMP280_Data_t bar_data;
  std::shared_ptr<I2C> hi2c;
  Status _device_status;
  Status reading_status;
  uint8_t address;
};

} // namespace stmepic::sensors::barometer