
#include "BNO055.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "stmepic.hpp"

using namespace stmepic::sensors::imu;


Result<std::shared_ptr<BNO055>> BNO055::Make(std::shared_ptr<I2C> hi2c, GpioPin *nreset, GpioPin *interrupt) {

  if(hi2c == nullptr)
    return Status::ExecutionError("I2C is nullpointer");
  auto a = std::shared_ptr<BNO055>(new BNO055(hi2c, nreset, interrupt));
  return Result<std::shared_ptr<BNO055>>::OK(a);
}
BNO055::BNO055(std::shared_ptr<I2C> hi2c, GpioPin *nreset, GpioPin *interrupt)

: hi2c(hi2c), interrupt(interrupt), nreset(nreset), _device_status(Status::Disconnected("not started")),
  reading_status(Status::OK()) {
}

Status BNO055::device_get_status() {
  return _device_status;
}


Status BNO055::device_stop() {
  if(nreset != nullptr)
    nreset->write(1);
  else {
    uint8_t reg = 0;

    set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0);
    STMEPIC_RETURN_ON_ERROR(hi2c->read(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_SYS_TRIGGER, &reg, 1));
    reg |= (1 << 5);
    STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_SYS_TRIGGER, &reg, 1));

    Ticker::get_instance().delay_nop(650);
  }
}


Status BNO055::device_start() {
  if(nreset != nullptr)
    nreset->write(1);

  STMEPIC_RETURN_ON_ERROR(set_operation_mode(internal::BNO055_OPR_MODE_t::BNO055_OPR_MODE_CONFIGMODE));
  STMEPIC_RETURN_ON_ERROR(set_page(internal::BNO055_PAGE_t::BNO055_PAGE_1));

  uint8_t acc_config = 0x02;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_ACC_SLEEP_CONFIG, &acc_config, 1));

  // dla każdego tak zrobić
  uint8_t mag_config = 0x0D;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_MAG_CONFIG, &mag_config, 1));
  uint8_t gyr_config_0 = 0x0B;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_GYR_CONFIG_0, &gyr_config_0, 1));
  uint8_t gyr_config_1 = 0x00;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_GYR_CONFIG_1, &gyr_config_1, 1));

  STMEPIC_RETURN_ON_ERROR(set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0));

  uint8_t unit_sel = 0x06;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_UNIT_SEL, &unit_sel, 1));
  uint8_t temp_source = 0x00;
  STMEPIC_RETURN_ON_ERROR(hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_TEMP_SOURCE, &temp_source, 1));

  STMEPIC_RETURN_ON_ERROR(set_operation_mode(internal::BNO055_OPR_MODE_t::BNO055_OPR_MODE_NDOF));

  Ticker::get_instance().delay_nop(50);

  // reset();
  // nie wiaodmo co z tym zrobić i dlaczego tutaj jest, jeśli nie będzie działać to możliwe, że jednak jest potrzebny ale wtedy

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
  uint8_t regs[45] = { 0 };

  STMEPIC_RETURN_ON_ERROR(set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0));
  STMEPIC_RETURN_ON_ERROR(hi2c->read(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_TEMP, regs, 45).status());

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

  return Result<BNO055_Data_t>::OK(data);
}

Result<BNO055_Data_t> BNO055::get_data() {
  return Result<BNO055_Data_t>::Propagate(imu_data, _device_status);
}

Result<bool> BNO055::device_is_connected() {
  return Result<bool>::OK(true);
}

Status BNO055::set_page(internal::BNO055_PAGE_t page) {
  return hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_PAGE_ID, (uint8_t *)&page, 1);
}

Status BNO055::set_operation_mode(internal::BNO055_OPR_MODE_t mode) {
  STMEPIC_RETURN_ON_ERROR(set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0));
  return hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_OPR_MODE, (uint8_t *)&mode, 1);
}

Status BNO055::set_power_mode(internal::BNO055_PWR_MODE_t mode) {
  STMEPIC_RETURN_ON_ERROR(set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0));
  return hi2c->write(internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_PWR_MODE, (uint8_t *)&mode, 1);
}
