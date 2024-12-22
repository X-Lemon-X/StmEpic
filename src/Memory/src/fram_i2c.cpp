
#include "fram_i2c.hpp"
#include "device.hpp"
#include "stmepic.hpp"
#include "stmepic_status.hpp"

using namespace stmepic::memory;
using namespace stmepic;


FramI2C::FramI2C(I2C_HandleTypeDef &_hi2c, uint8_t _device_address, uint16_t _begin_address, uint32_t _fram_size):
  hi2c(&_hi2c), device_address(_device_address), begin_address(_begin_address), fram_size(_fram_size)
{}


Status FramI2C::init()
{
  return device_get_status().status();
}

Result<DeviceStatus> FramI2C::device_get_status()
{
  if(hi2c == nullptr) return Status::Invalid("I2C is not initialized");
  auto status = HAL_I2C_IsDeviceReady(hi2c, device_address, 1, 100);
  return Result<DeviceStatus>::OK(DeviceTranslateStatus::translate_hal_status_to_device(status));
}

bool FramI2C::device_ok()
{
  auto status = device_get_status();
  if (!status.ok()) return false;
  return status.valueOrDie() == DeviceStatus::OK;
}

Result<bool> FramI2C::device_is_connected()
{
  auto status = device_get_status();
  return status.ok() ? Result<bool>::OK(true) : status.status();
}

Status FramI2C::device_reset()
{
  return device_get_status().status();
}

Status FramI2C::device_start()
{
  return device_get_status().status();
}

Status FramI2C::device_stop()
{
  return Status::OK();
}


Status FramI2C::write(uint32_t address, const std::vector<uint8_t> &data)
{
  STMEPIC_ASSING_OR_RETURN(encoded_data, FRAM::encode_data(data));
  if(encoded_data.size() > fram_size) return Status::CapacityError("Data is too big for the FRAM");
  auto status = HAL_I2C_Mem_Write(hi2c, device_address, begin_address + address, 1, encoded_data.data(), encoded_data.size(), 100);
  return DeviceTranslateStatus::translate_hal_status_to_status(status);
}


Result<std::vector<uint8_t>> FramI2C::read(uint32_t address)
{
  std::vector<uint8_t> data;
  uint8_t data_size[2];
  auto status = HAL_I2C_Mem_Read(hi2c, device_address, begin_address + address + 7 , 1, data_size, 2, 100);
  STMEPIC_RETURN_ON_ERROR(DeviceTranslateStatus::translate_hal_status_to_status(status));
  // if(status != HAL_OK) return Status::IOError("Error reading data size from FRAM");
  uint16_t size = data_size[0] | (data_size[1] << 8);
  data.resize(size+frame_size);
  status = HAL_I2C_Mem_Read(hi2c, device_address, begin_address + address, 1, data.data(), size+frame_size, 100);
  STMEPIC_RETURN_ON_ERROR(DeviceTranslateStatus::translate_hal_status_to_status(status));
  // if(status != HAL_OK) return Status::IOError("Error reading data from FRAM");
  return FRAM::decode_data(data);
}



FramI2CFM24CLxx::FramI2CFM24CLxx(I2C_HandleTypeDef &hi2c,uint16_t begin_address, uint32_t fram_size):
  FramI2C(hi2c, 0xA0, begin_address, fram_size)
{}

Status FramI2CFM24CLxx::write(uint32_t address, const std::vector<uint8_t> &data)
{
  auto mayby_encoded_data = FRAM::encode_data(data);
  if(!mayby_encoded_data.ok()) return mayby_encoded_data.status();
  auto encoded_data = mayby_encoded_data.valueOrDie();
  if(encoded_data.size() > fram_size) return Status::CapacityError("Data is too big for the FRAM");
  uint32_t memory_address = begin_address + address;
  uint16_t dev_address = device_address | ((memory_address >> 8 ) & 0x0E);

  auto status = HAL_I2C_Mem_Write(hi2c, dev_address, memory_address, 1, encoded_data.data(), encoded_data.size(), 200);
  
  return DeviceTranslateStatus::translate_hal_status_to_status(status);
}

Result<std::vector<uint8_t>> FramI2CFM24CLxx::read(uint32_t address)
{
  std::vector<uint8_t> data;
  uint8_t data_size[2];
  uint32_t memory_address = begin_address + address;
  uint16_t dev_address = device_address | ((memory_address >> 8 ) & 0x0E);
  auto status = HAL_I2C_Mem_Read(hi2c, dev_address, memory_address + 7 , 1, data_size, 2, 200);
  if(status != HAL_OK) return DeviceTranslateStatus::translate_hal_status_to_status(status);
  uint16_t size = (data_size[0] << 8) | data_size[1];
  data.resize(size+frame_size);
  status = HAL_I2C_Mem_Read(hi2c, dev_address, memory_address, 1, data.data(), size+frame_size, 200);
  if(status != HAL_OK) return DeviceTranslateStatus::translate_hal_status_to_status(status);
  return FRAM::decode_data(data);
}
