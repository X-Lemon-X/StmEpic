
#include "BNO055.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "stmepic.hpp"
#include "vectors3d.hpp"
#include "i2c.hpp"
#include <memory>
#include <cstring>
#include <vector>

using namespace stmepic::sensors::imu;
using namespace stmepic::sensors::imu::internal;


Result<std::shared_ptr<BNO055>> BNO055::Make(std::shared_ptr<I2C> hi2c, uint8_t address, GpioPin *nreset, GpioPin *interrupt) {

  if(hi2c == nullptr)
    return Status::ExecutionError("I2C is nullpointer");
  auto a = std::shared_ptr<BNO055>(new BNO055(hi2c, address, nreset, interrupt));
  return Result<std::shared_ptr<BNO055>>::OK(a);
}
BNO055::BNO055(std::shared_ptr<I2C> hi2c, uint8_t _address, GpioPin *nreset, GpioPin *interrupt)

: hi2c(hi2c), interrupt(interrupt), nreset(nreset), _device_status(Status::Disconnected("not started")),
  reading_status(Status::OK()), address(_address), imu_data({}),
  imu_settings(std::make_unique<BNO0055_Settings>()) {
}

Status BNO055::device_set_settings(const DeviceSettings &settings) {
  auto maybe_settings = dynamic_cast<const BNO0055_Settings *>(&settings);
  if(maybe_settings == nullptr) {
    return Status::ExecutionError("Settings are not of type BNO0055_Settings");
  }
  imu_settings = std::make_unique<BNO0055_Settings>(*maybe_settings);
  return Status::OK();
}

Status BNO055::device_get_status() {
  return _device_status;
}


Status BNO055::device_stop() {
  if(nreset != nullptr)
    nreset->write(1);
  else {
    uint8_t reg = 0;

    STMEPIC_RETURN_ON_ERROR(set_page(BNO055_PAGE_0));
    reg |= BNO055_SYS_TRIGGER_RESET;
    STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BNO055_REG_SYS_TRIGGER, &reg, 1));
    Ticker::get_instance().delay_nop(650);
  }
}


Status BNO055::device_start() {
  if(nreset != nullptr)
    nreset->write(1);

  STMEPIC_RETURN_ON_ERROR(hi2c->is_device_ready(address, 1, 500));
  STMEPIC_RETURN_ON_ERROR(set_page(BNO055_PAGE_0));
  uint8_t data[4] = {};
  STMEPIC_RETURN_ON_ERROR(hi2c->read(address, BNO055_REG_CHIP_ID, data, 4));
  bool correct_chip = data[0] == internal::BNO055_CHIP_ID;
  correct_chip &= data[1] == internal::BNO055_ACC_ID;
  correct_chip &= data[2] == internal::BNO055_MAG_ID;
  correct_chip &= data[3] == internal::BNO055_GYRO_ID;
  if(!correct_chip) {
    _device_status = Status::Disconnected("BNO055 is not recognized");
    return _device_status;
  }

  // we set the use of external crystal and reset the device
  uint8_t reg;
  reg = BNO055_SYS_TRIGGER_RESET | BNO055_SYS_TRIGGER_EXT_CRYSTAL;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BNO055_REG_SYS_TRIGGER, &reg, 1));
  Ticker::get_instance().delay_nop(650);

  // if user provided calibration data we set it
  if(imu_settings->calibration_data.calibrated) {
    STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BNO055_REG_CALIBRATION_DATA,
                                        imu_settings->calibration_data.data, BNO055_CALIBRATION_DATA_LENGTH));
  }

  // set the units to some normal ones
  reg = BNO055_UNIT_SEL_TEMP_Unit_C | BNO055_UNIT_SEL_EUL_Unit_Rad | BNO055_UNIT_SEL_GYR_Unit_RPS;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BNO055_REG_UNIT_SEL, &reg, 1));

  // set the operation mode to Fusion NDOF
  reg = BNO055_OPR_MODE_NDOF;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BNO055_REG_OPR_MODE, &reg, 1));

  // wait for the device to be ready
  Ticker::get_instance().delay_nop(25);


  // at this point the imu should be ready to use and be in NDOF mode ready to read data from it
  return Status::OK();
}

Status BNO055::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  return device_start();
}

Status BNO055::do_device_task_start() {
  return DeviceThreadedBase::do_default_task_start(task_imu, task_imu_before, this);
}

Status BNO055::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}

BNO055_Calibration_Data_t BNO055::get_calibration_data() {
  return imu_settings->calibration_data;
}

void BNO055::task_imu_before(SimpleTask &handler, void *arg) {
  (void)handler;
  BNO055 *imu = static_cast<BNO055 *>(arg);
  imu->device_start();
}

void BNO055::task_imu(SimpleTask &handler, void *arg) {
  (void)handler;
  BNO055 *imu = static_cast<BNO055 *>(arg);
  imu->handle();
}


bool BNO055::device_ok() {
  return _device_status.ok();
}

void BNO055::handle() {
  auto maybe_data = read_data();
  if(maybe_data.ok()) {
    imu_data = maybe_data.valueOrDie();
  } else if(maybe_data.status().status_code() == StatusCode::HalBusy) {
    hi2c->hardware_reset();
    vTaskDelay(10);
  }
  _device_status = maybe_data.status();
}

Result<BNO055_Data_t> BNO055::read_data() {
  uint8_t regs[BNO055_REG_ACC_DATA_LENGTH + 1] = { 0 };

  STMEPIC_RETURN_ON_ERROR(hi2c->read(address, BNO055_REG_ACC_DATA_BEGIN, regs, sizeof(regs)));

  BNO055_Data_t data = {};

  data.acc.x = ((uint16_t)regs[1] << 8) | regs[0];
  data.acc.y = ((uint16_t)regs[3] << 8) | regs[2];
  data.acc.z = ((uint16_t)regs[5] << 8) | regs[4];

  data.mag.x = ((uint16_t)regs[7] << 8) | regs[6];
  data.mag.y = ((uint16_t)regs[9] << 8) | regs[8];
  data.mag.z = ((uint16_t)regs[11] << 8) | regs[10];

  data.gyr.x = ((uint16_t)regs[13] << 8) | regs[12];
  data.gyr.y = ((uint16_t)regs[15] << 8) | regs[14];
  data.gyr.z = ((uint16_t)regs[17] << 8) | regs[16];

  data.eul.x = ((uint16_t)regs[19] << 8) | regs[18];
  data.eul.y = ((uint16_t)regs[21] << 8) | regs[20];
  data.eul.z = ((uint16_t)regs[23] << 8) | regs[22];

  data.qua.w = ((uint16_t)regs[25] << 8) | regs[24];
  data.qua.x = ((uint16_t)regs[27] << 8) | regs[26];
  data.qua.y = ((uint16_t)regs[29] << 8) | regs[28];
  data.qua.z = ((uint16_t)regs[31] << 8) | regs[30];

  data.lia.x = ((uint16_t)regs[33] << 8) | regs[32];
  data.lia.y = ((uint16_t)regs[35] << 8) | regs[34];
  data.lia.z = ((uint16_t)regs[37] << 8) | regs[36];

  data.grv.x = ((uint16_t)regs[39] << 8) | regs[38];
  data.grv.y = ((uint16_t)regs[41] << 8) | regs[40];
  data.grv.z = ((uint16_t)regs[43] << 8) | regs[42];

  data.temp = regs[44];

  // if sensor is calibrated we simply return the data
  if(imu_settings->calibration_data.calibrated)
    return Result<BNO055_Data_t>::OK(data);

  // if sensor is not calibrated we need to check the calibration status
  bool calib_stat_gyro = ((regs[45] >> 6) & 0x03) == 3;
  bool calib_stat_acc  = ((regs[45] >> 4) & 0x03) == 3;
  bool calib_stat_mag  = ((regs[45] >> 2) & 0x03) == 3;
  bool calib_stat_sys  = (regs[45] & 0x03) == 3;


  imu_settings->calibration_data.calibrated = calib_stat_gyro && calib_stat_acc && calib_stat_mag && calib_stat_sys;

  // if calibration is done we need to read the calibration data
  if(imu_settings->calibration_data.calibrated) {
    uint8_t cal_reg[BNO055_CALIBRATION_DATA_LENGTH] = {};
    STMEPIC_RETURN_ON_ERROR(hi2c->read(address, BNO055_REG_CALIBRATION_DATA, imu_settings->calibration_data.data,
                                       sizeof(imu_settings->calibration_data.data)));
  }
  return Result<BNO055_Data_t>::OK(data);
}

Result<BNO055_Data_t> BNO055::get_data() {
  return Result<BNO055_Data_t>::Propagate(imu_data, _device_status);
}

Result<bool> BNO055::device_is_connected() {
  return Result<bool>::OK(true);
}

Status BNO055::set_page(uint8_t page) {
  return hi2c->write(address, internal::BNO055_REG_PAGE, (uint8_t *)&page, 1);
}

// Status BNO055::set_operation_mode(internal::BNO055_OPR_MODE_t mode) {
//   STMEPIC_RETURN_ON_ERROR(set_page(BNO055_PAGE_0));
//   return hi2c->write(address, internal::BNO055_REG_OPR_MODE, (uint8_t *)&mode, 1);
// }

// Status BNO055::set_power_mode(internal::BNO055_PWR_MODE_t mode) {
//   STMEPIC_RETURN_ON_ERROR(set_page(BNO055_PAGE_0));
//   return hi2c->write(address, internal::BNO055_REG_PWR_MODE, (uint8_t *)&mode, 1);
// }
