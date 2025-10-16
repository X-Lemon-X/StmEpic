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
 * @file BNO055.hpp
 * @brief  BNO055 IMU sensor class definition.
 *
 */

/**
 * @defgroup Sensors
 * @brief Functions related to different sensors.
 * @{
 */


/**
 * @defgroup imu_sensors IMU
 * @brief Functions related to different IMU sensors.
 * @{
 */

/**
 * @defgroup BNO055_imu_sensors BNO055
 * @brief BNO055 IMU sensors.
 * @{
 */

namespace stmepic::sensors::imu::internal {

static const uint8_t BNO055_I2C_ADDRESS_1 = 0x28;
static const uint8_t BNO055_I2C_ADDRESS_2 = 0x29;

static const uint8_t BNO055_REG_CHIP_ID             = 0x00; // beginning reg for who am i
static const uint8_t BNO055_CHIP_ID                 = 0xA0; // BNO055 chip id
static const uint8_t BNO055_REG_PAGE                = 0x07; // select page reg
static const uint8_t BNO055_PAGE_0                  = 0x00; // select page reg
static const uint8_t BNO055_PAGE_1                  = 0x01; // select page reg
static const uint8_t BNO055_REG_ACC_CHIP_ID         = 0x01; // reg for acc chip id
static const uint8_t BNO055_REG_MAG_CHIP_ID         = 0x02; // reg for mag chip id
static const uint8_t BNO055_REG_GYRO_CHIP_ID        = 0x03;
static const uint8_t BNO055_ACC_ID                  = 0xFB;
static const uint8_t BNO055_MAG_ID                  = 0x32;
static const uint8_t BNO055_GYRO_ID                 = 0x0F;
static const uint8_t BNO055_REG_ST_RESULT           = 0x36;
static const uint8_t BNO055_REG_CALIB_STAT          = 0x35;
static const uint8_t BNO055_REG_SYS_TRIGGER         = 0x3F;
static const uint8_t BNO055_SYS_TRIGGER_RESET       = 0x20;
static const uint8_t BNO055_SYS_TRIGGER_EXT_CRYSTAL = 0x80;

// operation mode config
static const uint8_t BNO055_REG_OPR_MODE        = 0x3D;
static const uint8_t BNO055_OPR_MODE_NDOF       = 0x0C;
static const uint8_t BNO055_OPR_MODE_CONFIGMODE = 0x0;


static const uint8_t BNO055_REG_UNIT_SEL = 0x3B;

// acc m/s^2, gyro rad/s, mag uT, euler rad, temp C,
// Data format: Windows
static const uint8_t BNO055_UNIT_SEL_ORI_And_Win_WIN = 0x80;
static const uint8_t BNO055_UNIT_SEL_TEMP_Unit_C     = 0x10;
static const uint8_t BNO055_UNIT_SEL_EUL_Unit_Rad    = 0x04;
static const uint8_t BNO055_UNIT_SEL_GYR_Unit_RPS    = 0x02;
static const uint8_t BNO055_UNIT_SEL_ACC_Unit_MG     = 0x01;

// beginning reg for acceleration  an other data that can be read
static const uint8_t BNO055_REG_ACC_DATA_BEGIN  = 0x08;
static const uint8_t BNO055_REG_ACC_DATA_LENGTH = 45; // length of acceleration data


static const uint8_t BNO055_REG_CALIBRATION_DATA    = 0x43;
static const uint8_t BNO055_CALIBRATION_DATA_LENGTH = 28;


} // namespace stmepic::sensors::imu::internal

namespace stmepic::sensors::imu {


/// @brief BNO055 calibration reg starting with SIC_MATRIX_LSB0
/// calibration data consist of SIC_MATRIX and Offset of: Accelerometer, Magnetometer and Gyroscope.
/// and Radius of: Accelerometerm and Magnetometer.
struct BNO055_Calibration_Data_t {
  bool calibrated;
  uint8_t data[internal::BNO055_CALIBRATION_DATA_LENGTH];
};

struct BNO055_Data_t {
  int8_t temp;
  Vector3d_t<int16_t> acc;
  Vector3d_t<int16_t> gyr;
  Vector3d_t<int16_t> mag;
  Vector3d_t<int16_t> lia;
  Vector3d_t<int16_t> grv;
  Vector3d_t<int16_t> eul;
  Vector4d_t<int16_t> qua;
};

struct BNO0055_Settings : public DeviceSettings {
  BNO055_Calibration_Data_t calibration_data = {};
};

/**
 * @brief BNO055 IMU sensor
 * BNO055 is a 9-axis IMU sensor with a built-in microcontroller that can perform sensor fusion
 */
class BNO055 : public stmepic::DeviceThreadedBase {
public:
  /**
   * @brief Make new BNO055 IMU sensor object
   *
   * @param hi2c the I2cBase handle that will be used to communicate with the BNO055 device
   * @param address the address of the BNO055 device one of two possible addresses
   * @param nreset the reset pin of the BNO055 device
   * @param interrupt the interrupt pin of the BNO055 device
   * @return Brand new BNO055 object
   */
  static Result<std::shared_ptr<BNO055>> Make(std::shared_ptr<I2cBase> hi2c,
                                              uint8_t address    = internal::BNO055_I2C_ADDRESS_1,
                                              GpioPin *nreset    = nullptr,
                                              GpioPin *interrupt = nullptr);

  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;
  Status device_set_settings(const DeviceSettings &settings) override;

  /**
   * @brief Get the last read data from the BNO055 sensor
   * @return This function will return the last read data from the BNO055 sensor along with the status of the last operation
   */
  Result<BNO055_Data_t> get_data();

  BNO055_Calibration_Data_t get_calibration_data();
  // void set_calibration_data(BNO055_Calibration_Data_t &calibration_data);


private:
  BNO055(std::shared_ptr<I2cBase> hi2c, uint8_t address, GpioPin *nreset = nullptr, GpioPin *interrupt = nullptr);
  Status do_device_task_start() override;
  Status do_device_task_stop() override;
  Status do_device_task_reset() override;

  Status init();
  Status stop();


  Result<BNO055_Data_t> read_data();
  static Status task_imu_before(SimpleTask &handler, void *arg);
  static Status task_imu(SimpleTask &handler, void *arg);
  Status handle();
  // Status set_operation_mode(internal::BNO055_OPR_MODE_t mode);
  // Status set_power_mode(internal::BNO055_PWR_MODE_t mode);
  Status set_page(uint8_t page);

  Status device_init();

  BNO055_Data_t imu_data;
  std::shared_ptr<I2cBase> hi2c;
  GpioPin *interrupt;
  GpioPin *nreset;

  std::unique_ptr<BNO0055_Settings> imu_settings;


  uint8_t address;
  Status _device_status;
  Status reading_status;
};

} // namespace stmepic::sensors::imu