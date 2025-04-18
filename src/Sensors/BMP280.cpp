
#include "stmepic.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "BMP280.hpp"

using namespace stmepic::sensors::barometer;
using namespace stmepic::sensors::barometer::internal;
using namespace stmepic;


Result<std::shared_ptr<BMP280>> BMP280::Make(std::shared_ptr<I2C> hi2c, uint8_t address) {
  if(hi2c == nullptr)
    return Status::ExecutionError("I2C is nullpointer");
  auto a = std::shared_ptr<BMP280>(new BMP280(hi2c, address));
  return Result<std::shared_ptr<BMP280>>::OK(a);
}

BMP280::BMP280(std::shared_ptr<I2C> hi2c, uint8_t _address)

: hi2c(hi2c), _device_status(Status::Disconnected("not started")), reading_status(Status::OK()), address(_address) {
  if(hi2c == nullptr)
    _device_status = Status::ExecutionError("I2C is nullpointer");
  else
    _device_status = Status::OK();
}

Status BMP280::device_get_status() {
  return _device_status;
}


Status BMP280::device_stop() {
  uint8_t data = BMP280_RESET_VALUE;
  return hi2c->write(address, BMP280_REG_RESET, &data, 1);
}

Status BMP280::device_start() {
  STMEPIC_ASSING_TO_OR_RETURN(_device_status, hi2c->is_device_ready(address, 1, 500));
  uint8_t data[24] = {};

  // read chip id
  STMEPIC_ASSING_TO_OR_RETURN(_device_status, hi2c->read(address, BMP280_REG_CHIP_ID, data, 1));
  if(data[0] != internal::BMP280_CHIP_ID) {
    _device_status = Status::ExecutionError("BMP280 chip id is not correct");
    return _device_status;
  }

  uint8_t ctrl_meas = 0b01011111; // sets sampling to 16x, ans starts normal mode
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BMP280_REG_CTRL_MEAS, &ctrl_meas, 1));

  uint8_t config = 0b00011100; // sets standby mode  to 0.5ms and filte to 16
  STMEPIC_RETURN_ON_ERROR(hi2c->write(address, BMP280_REG_CONFIG, &config, 1));


  // read all calibration data from the device from dig_T1 to dig_P9 |  0x88 to 0x9F   24 bytes
  STMEPIC_ASSING_TO_OR_RETURN(_device_status, hi2c->read(address, BMP280_REG_DIG_T1, data, 24));
  dig_T1 = (data[1] << 8) | data[0];
  dig_T2 = (data[3] << 8) | data[2];
  dig_T3 = (data[5] << 8) | data[4];

  dig_P1 = (data[7] << 8) | data[6];
  dig_P2 = (data[9] << 8) | data[8];
  dig_P3 = (data[11] << 8) | data[10];
  dig_P4 = (data[13] << 8) | data[12];
  dig_P5 = (data[15] << 8) | data[14];
  dig_P6 = (data[17] << 8) | data[16];
  dig_P7 = (data[19] << 8) | data[18];
  dig_P8 = (data[21] << 8) | data[20];
  dig_P9 = (data[23] << 8) | data[22];

  return Status::OK();
}

Status BMP280::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  return device_start();
}

Status BMP280::do_device_task_start() {
  return DeviceThreadedBase::do_default_task_start(task_bar, task_bar_before, this);
}

Status BMP280::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}

bool BMP280::device_ok() {
  return _device_status.ok();
}

Result<bool> BMP280::device_is_connected() {
  return Result<bool>::Propagate(_device_status.ok(), _device_status);
}


void BMP280::task_bar_before(SimpleTask &handler, void *arg) {
  (void)handler;
  BMP280 *bar = static_cast<BMP280 *>(arg);
  (void)bar->device_start();
}

void BMP280::task_bar(SimpleTask &handler, void *arg) {
  (void)handler;
  BMP280 *imu = static_cast<BMP280 *>(arg);
  imu->handle();
}

void BMP280::handle() {
  auto maybe_data = read_data();
  if(maybe_data.ok()) {
    bar_data = maybe_data.valueOrDie();
  } else if(maybe_data.status().status_code() == StatusCode::HalBusy) {
    hi2c->hardware_reset();
    vTaskDelay(10);
  }
  _device_status = maybe_data.status();
}

float BMP280::bmp280_compensate_T_int32(int32_t adc_T) {
  int32_t var1, var2, T;
  var1   = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
  var2   = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
  t_fine = var1 + var2;
  T      = (t_fine * 5 + 128) >> 8;
  return (float)T / 100.0f;
}

float BMP280::bmp280_compensate_P_int64(int32_t adc_P) {
  int64_t var1, var2, p;
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)dig_P6;
  var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
  var2 = var2 + (((int64_t)dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
  if(var1 == 0) {
    return 0.0f; // avoid exception caused by division by zero
  }
  p    = 1048576 - adc_P;
  p    = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)dig_P8) * p) >> 19;
  p    = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
  return (float)p / 25600.0f;
}

float BMP280::bmp280_compensate_P_int32(int32_t adc_P) {
  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
  var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
  var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
  var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
  if(var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
  if(p < 0x80000000) {
    p = (p << 1) / ((uint32_t)var1);
  } else {
    p = (p / (uint32_t)var1) * 2;
  }
  var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
  var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
  p    = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
  return (float)p / 100.0f;
}

Result<BMP280_Data_t> BMP280::read_data() {
  uint8_t regs[6] = {};
  STMEPIC_ASSING_OR_RETURN(_device_status, hi2c->read(address, BMP280_REG_PRES_MSB, regs, 6));

  int32_t adc_T = ((uint32_t)regs[3] << 12) | ((uint32_t)regs[4] << 4) | ((uint32_t)regs[5] >> 4);
  int32_t adc_P = ((uint32_t)regs[0] << 12) | ((uint32_t)regs[1] << 4) | ((uint32_t)regs[2] >> 4);
  BMP280_Data_t data;
  data.temp     = bmp280_compensate_T_int32(adc_T);
  data.pressure = bmp280_compensate_P_int32(adc_P);
  return Result<BMP280_Data_t>::OK(data);
}

Result<BMP280_Data_t> BMP280::get_data() {
  return Result<BMP280_Data_t>::Propagate(bar_data, _device_status);
}
