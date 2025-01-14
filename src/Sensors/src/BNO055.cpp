
#include "stmepic.hpp"
#include "pin.hpp"
#include "device.hpp"
#include "BNO055.hpp"

using namespace stmepic::sensors::imu;


BNO055::BNO055(I2C_HandleTypeDef& i2c, gpio::GpioPin *reset) : i2c(i2c), reset(reset) {
  device_reset();
}

Status BNO055::set_page(internal::BNO055_PAGE_t page){
  auto status = HAL_I2C_Mem_Write(&i2c, internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_PAGE_ID, 1, (uint8_t*)&page, 1, 100);
  return stmepic::DeviceTranslateStatus::translate_hal_status_to_status(status);
}

Status BNO055::set_operation_mode(internal::BNO055_OPR_MODE_t mode){
  set_page(internal::BNO055_PAGE_t::BNO055_PAGE_0);
  auto status = HAL_I2C_Mem_Write(&i2c, internal::BNO055_I2C_ADDRESS, internal::BNO055_REG_OPR_MODE, 1, (uint8_t*)&mode, 1, 100);
  return stmepic::DeviceTranslateStatus::translate_hal_status_to_status(status);
}

Status BNO055::device_start(){

}