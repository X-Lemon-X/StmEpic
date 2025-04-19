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
 * @brief IMU_Sensors IMU sensors
 * @defgroup Function related to different IMU sensors
 *
 */

namespace stmepic::sensors::imu::internal {

static const uint8_t BNO055_I2C_ADDRESS        = 0x29; // 0x28 << 1
static const uint8_t BNO055_REG_MAG_RADIUS_MSB = 0x6A;
static const uint8_t BNO055_REG_MAG_RADIUS_LSB = 0x69;
static const uint8_t BNO055_REG_ACC_RADIUS_MSB = 0x68;
static const uint8_t BNO055_REG_ACC_RADIUS_LSB = 0x67;

static const uint8_t BNO055_REG_GYR_OFFSET_Z_MSB = 0x66;
static const uint8_t BNO055_REG_GYR_OFFSET_Z_LSB = 0x65;
static const uint8_t BNO055_REG_GYR_OFFSET_Y_MSB = 0x64;
static const uint8_t BNO055_REG_GYR_OFFSET_Y_LSB = 0x63;
static const uint8_t BNO055_REG_GYR_OFFSET_X_MSB = 0x62;
static const uint8_t BNO055_REG_GYR_OFFSET_X_LSB = 0x61;

static const uint8_t BNO055_REG_MAG_OFFSET_Z_MSB = 0x60;
static const uint8_t BNO055_REG_MAG_OFFSET_Z_LSB = 0x5F;
static const uint8_t BNO055_REG_MAG_OFFSET_Y_MSB = 0x5E;
static const uint8_t BNO055_REG_MAG_OFFSET_Y_LSB = 0x5D;
static const uint8_t BNO055_REG_MAG_OFFSET_X_MSB = 0x5C;
static const uint8_t BNO055_REG_MAG_OFFSET_X_LSB = 0x5B;

static const uint8_t BNO055_REG_ACC_OFFSET_Z_MSB = 0x5A;
static const uint8_t BNO055_REG_ACC_OFFSET_Z_LSB = 0x59;
static const uint8_t BNO055_REG_ACC_OFFSET_Y_MSB = 0x58;
static const uint8_t BNO055_REG_ACC_OFFSET_Y_LSB = 0x57;
static const uint8_t BNO055_REG_ACC_OFFSET_X_MSB = 0x56;
static const uint8_t BNO055_REG_ACC_OFFSET_X_LSB = 0x55;

static const uint8_t BNO055_REG_AXIS_MAP_SIGN   = 0x42;
static const uint8_t BNO055_REG_AXIS_MAP_CONFIG = 0x41;
static const uint8_t BNO055_REG_TEMP_SOURCE     = 0x40;
static const uint8_t BNO055_REG_SYS_TRIGGER     = 0x3F;
static const uint8_t BNO055_REG_PWR_MODE        = 0x3E;
static const uint8_t BNO055_REG_OPR_MODE        = 0x3D;
static const uint8_t BNO055_REG_UNIT_SEL        = 0x3B;
static const uint8_t BNO055_REG_SYS_ERR         = 0x3A;
static const uint8_t BNO055_REG_SYS_STATUS      = 0x39;
static const uint8_t BNO055_REG_SYS_CLK_STATUS  = 0x38;
static const uint8_t BNO055_REG_INT_STA         = 0x37;
static const uint8_t BNO055_REG_ST_RESULT       = 0x36;
static const uint8_t BNO055_REG_CALIB_STAT      = 0x35;

static const uint8_t BNO055_REG_TEMP = 0x34;

static const uint8_t BNO055_REG_GRV_DATA_Z_MSB = 0x33;
static const uint8_t BNO055_REG_GRV_DATA_Z_LSB = 0x32;
static const uint8_t BNO055_REG_GRV_DATA_Y_MSB = 0x31;
static const uint8_t BNO055_REG_GRV_DATA_Y_LSB = 0x30;
static const uint8_t BNO055_REG_GRV_DATA_X_MSB = 0x2F;
static const uint8_t BNO055_REG_GRV_DATA_X_LSB = 0x2E;

static const uint8_t BNO055_REG_LIA_DATA_Z_MSB = 0x2D;
static const uint8_t BNO055_REG_LIA_DATA_Z_LSB = 0x2C;
static const uint8_t BNO055_REG_LIA_DATA_Y_MSB = 0x2B;
static const uint8_t BNO055_REG_LIA_DATA_Y_LSB = 0x2A;
static const uint8_t BNO055_REG_LIA_DATA_X_MSB = 0x29;
static const uint8_t BNO055_REG_LIA_DATA_X_LSB = 0x28;

static const uint8_t BNO055_REG_QUA_DATA_Z_MSB = 0x27;
static const uint8_t BNO055_REG_QUA_DATA_Z_LSB = 0x26;
static const uint8_t BNO055_REG_QUA_DATA_Y_MSB = 0x25;
static const uint8_t BNO055_REG_QUA_DATA_Y_LSB = 0x24;
static const uint8_t BNO055_REG_QUA_DATA_X_MSB = 0x23;
static const uint8_t BNO055_REG_QUA_DATA_X_LSB = 0x22;
static const uint8_t BNO055_REG_QUA_DATA_W_MSB = 0x21;
static const uint8_t BNO055_REG_QUA_DATA_W_LSB = 0x20;

static const uint8_t BNO055_REG_EUL_PITCH_MSB   = 0x1F;
static const uint8_t BNO055_REG_EUL_PITCH_LSB   = 0x1E;
static const uint8_t BNO055_REG_EUL_ROLL_MSB    = 0x1D;
static const uint8_t BNO055_REG_EUL_ROLL_LSB    = 0x1C;
static const uint8_t BNO055_REG_EUL_HEADING_MSB = 0x1B;
static const uint8_t BNO055_REG_EUL_HEADING_LSB = 0x1A;

static const uint8_t BNO055_REG_GYR_DATA_Z_MSB = 0x19;
static const uint8_t BNO055_REG_GYR_DATA_Z_LSB = 0x18;
static const uint8_t BNO055_REG_GYR_DATA_Y_MSB = 0x17;
static const uint8_t BNO055_REG_GYR_DATA_Y_LSB = 0x16;
static const uint8_t BNO055_REG_GYR_DATA_X_MSB = 0x15;
static const uint8_t BNO055_REG_GYR_DATA_X_LSB = 0x14;

static const uint8_t BNO055_REG_MAG_DATA_Z_MSB = 0x13;
static const uint8_t BNO055_REG_MAG_DATA_Z_LSB = 0x12;
static const uint8_t BNO055_REG_MAG_DATA_Y_MSB = 0x11;
static const uint8_t BNO055_REG_MAG_DATA_Y_LSB = 0x10;
static const uint8_t BNO055_REG_MAG_DATA_X_MSB = 0x0F;
static const uint8_t BNO055_REG_MAG_DATA_X_LSB = 0x0E;

static const uint8_t BNO055_REG_ACC_DATA_Z_MSB = 0x0D;
static const uint8_t BNO055_REG_ACC_DATA_Z_LSB = 0x0C;
static const uint8_t BNO055_REG_ACC_DATA_Y_MSB = 0x0B;
static const uint8_t BNO055_REG_ACC_DATA_Y_LSB = 0x0A;
static const uint8_t BNO055_REG_ACC_DATA_X_MSB = 0x09;
static const uint8_t BNO055_REG_ACC_DATA_X_LSB = 0x08;

static const uint8_t BNO055_REG_PAGE_ID       = 0x07;
static const uint8_t BNO055_REG_BL_REV_ID     = 0x06;
static const uint8_t BNO055_REG_SW_REV_ID_MSB = 0x05;
static const uint8_t BNO055_REG_SW_REV_ID_LSB = 0x04;
static const uint8_t BNO055_REG_GYR_ID        = 0x03;
static const uint8_t BNO055_REG_MAG_ID        = 0x02;
static const uint8_t BNO055_REG_ACC_ID        = 0x01;
static const uint8_t BNO055_REG_CHIP_ID       = 0x00;

static const uint8_t BNO055_REG_GYR_AM_SET     = 0x1F;
static const uint8_t BNO055_REG_GYR_AM_THRES   = 0x1E;
static const uint8_t BNO055_REG_GYR_DUR_Z      = 0x1D;
static const uint8_t BNO055_REG_GYR_HR_Z_SET   = 0x1C;
static const uint8_t BNO055_REG_GYR_DUR_Y      = 0x1B;
static const uint8_t BNO055_REG_GYR_HR_Y_SET   = 0x1A;
static const uint8_t BNO055_REG_GYR_DUR_X      = 0x19;
static const uint8_t BNO055_REG_GYR_HR_X_SET   = 0x18;
static const uint8_t BNO055_REG_GYR_INT_SETING = 0x17;

static const uint8_t BNO055_REG_ACC_NM_SET       = 0x16;
static const uint8_t BNO055_REG_ACC_NM_THRE      = 0x15;
static const uint8_t BNO055_REG_ACC_HG_THRES     = 0x14;
static const uint8_t BNO055_REG_ACC_HG_DURATION  = 0x13;
static const uint8_t BNO055_REG_ACC_INT_SETTINGS = 0x12;
static const uint8_t BNO055_REG_ACC_AM_THRES     = 0x11;

static const uint8_t BNO055_REG_INT_EN  = 0x10;
static const uint8_t BNO055_REG_INT_MSK = 0x0F;

static const uint8_t BNO055_REG_GYR_SLEEP_CONFIG = 0x0D;
static const uint8_t BNO055_REG_ACC_SLEEP_CONFIG = 0x0C;

static const uint8_t BNO055_REG_GYR_CONFIG_1 = 0x0B;
static const uint8_t BNO055_REG_GYR_CONFIG_0 = 0x0A;
static const uint8_t BNO055_REG_MAG_CONFIG   = 0x09;
static const uint8_t BNO055_REG_ACC_CONFIG   = 0x08;

enum class BNO055_PWR_MODE_t { BNO055_PWR_MODE_NORMAL, BNO055_PWR_MODE_LOW_POWER, BNO055_PWR_MODE_SUSPEND };

enum class BNO055_OPR_MODE_t {
  BNO055_OPR_MODE_CONFIGMODE,
  BNO055_OPR_MODE_ACCONLY,
  BNO055_OPR_MODE_MAGONLY,
  BNO055_OPR_MODE_GYROONLY,
  BNO055_OPR_MODE_ACCMAG,
  BNO055_OPR_MODE_ACCGYRO,
  BNO055_OPR_MODE_MAGGYRO,
  BNO055_OPR_MODE_AMG,
  BNO055_OPR_MODE_IMU,
  BNO055_OPR_MODE_COMPASS,
  BNO055_OPR_MODE_M4G,
  BNO055_OPR_MODE_NDOF_FMC_OFF,
  BNO055_OPR_MODE_NDOF
};

enum class BNO055_PAGE_t { BNO055_PAGE_0, BNO055_PAGE_1 };
} // namespace stmepic::sensors::imu::internal

namespace stmepic::sensors::imu {

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

/**
 * @brief BNO055 IMU sensor
 * BNO055 is a 9-axis IMU sensor with a built-in microcontroller that can perform sensor fusion
 */
class BNO055 : public stmepic::DeviceThreadedBase {
public:
  /**
   * @brief Make new BNO055 IMU sensor object
   *
   * @param hi2c the I2C handle that will be used to communicate with the BNO055 device
   * @param nreset the reset pin of the BNO055 device
   * @param interrupt the interrupt pin of the BNO055 device
   * @return Brand new BNO055 object
   */
  static Result<std::shared_ptr<BNO055>>
  Make(std::shared_ptr<I2C> hi2c, GpioPin *nreset = nullptr, GpioPin *interrupt = nullptr);

  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;
  Status device_reset() override;
  Status device_start() override;
  Status device_stop() override;

  /**
   * @brief Get the last read data from the BNO055 sensor
   * @return This function will return the last read data from the BNO055 sensor along with the status of the last operation
   */
  Result<BNO055_Data_t> get_data();


private:
  BNO055(std::shared_ptr<I2C> hi2c, GpioPin *nreset = nullptr, GpioPin *interrupt = nullptr);
  stmepic::Status do_device_task_start() override;
  stmepic::Status do_device_task_stop() override;

  Result<BNO055_Data_t> read_data();
  static void task_imu_before(SimpleTask &handler, void *arg);
  static void task_imu(SimpleTask &handler, void *arg);
  void handle();
  Status set_operation_mode(internal::BNO055_OPR_MODE_t mode);
  Status set_power_mode(internal::BNO055_PWR_MODE_t mode);
  Status set_page(internal::BNO055_PAGE_t page);
  void setPage(internal::BNO055_PAGE_t page);


  Status device_init();

  BNO055_Data_t imu_data;
  std::shared_ptr<I2C> hi2c;
  GpioPin *interrupt;
  GpioPin *nreset;

  Status _device_status;
  Status reading_status;
};

} // namespace stmepic::sensors::imu