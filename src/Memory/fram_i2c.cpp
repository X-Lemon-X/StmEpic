
#include "fram_i2c.hpp"
#include "device.hpp"
#include "stmepic.hpp"
#include "status.hpp"

using namespace stmepic::memory;
using namespace stmepic;


Result<std::shared_ptr<FramI2C>>
FramI2C::Make(std::shared_ptr<I2C> _hi2c, uint8_t _device_address, uint16_t _begin_address, uint32_t _fram_size) {
  if(_hi2c == nullptr)
    return Status::Invalid("I2C is not initialized");
  if(_device_address == 0)
    return Status::Invalid("Device address is not valid");
  if(_begin_address > _fram_size)
    return Status::Invalid("Begin address is not valid");
  std::shared_ptr<FramI2C> fram(new FramI2C(_hi2c, _device_address, _begin_address, _fram_size));
  return Result<decltype(fram)>::OK(fram);
}

FramI2C::FramI2C(std::shared_ptr<I2C> _hi2c, uint8_t _device_address, uint16_t _begin_address, uint32_t _fram_size)
: hi2c(_hi2c), device_address(_device_address), begin_address(_begin_address), fram_size(_fram_size) {
}

FramI2C::~FramI2C() {
  device_stop();
}

Status FramI2C::device_get_status() {
  if(hi2c == nullptr)
    return Status::Invalid("I2C is not initialized");
  return hi2c->is_device_ready(device_address, 1, 100);
}

bool FramI2C::device_ok() {
  auto status = device_get_status();
  if(!status.ok())
    return false;
  return status.ok();
}

Result<bool> FramI2C::device_is_connected() {
  auto status = device_get_status();
  return status.ok() ? Result<bool>::OK(true) : status.status();
}

Status FramI2C::device_reset() {
  return device_get_status().status();
}

Status FramI2C::device_start() {
  return device_get_status().status();
}

Status FramI2C::device_stop() {
  return Status::OK();
}


Status FramI2C::write(uint32_t address, const std::vector<uint8_t> &data) {
  STMEPIC_ASSING_OR_RETURN(encoded_data, FRAM::encode_data(data));
  if(encoded_data.size() > fram_size)
    return Status::CapacityError("Data is too big for the FRAM");
  return hi2c->write(device_address, begin_address + address, encoded_data.data(), encoded_data.size());
}


Result<std::vector<uint8_t>> FramI2C::read(uint32_t address) {
  std::vector<uint8_t> data;
  uint8_t data_size[2];
  STMEPIC_RETURN_ON_ERROR(hi2c->read(device_address, begin_address + address + 7, data_size, 2));
  uint16_t size = data_size[0] | (data_size[1] << 8);
  data.resize(size + frame_size);
  STMEPIC_RETURN_ON_ERROR(hi2c->read(device_address, begin_address + address, data.data(), size + frame_size));
  return FRAM::decode_data(data);
}

Result<std::shared_ptr<FramI2CFM24CLxx>>
FramI2CFM24CLxx::Make(std::shared_ptr<I2C> _hi2c, uint16_t _begin_address, uint32_t _fram_size) {
  if(_hi2c == nullptr)
    return Status::Invalid("I2C is not initialized");
  if(_begin_address > _fram_size)
    return Status::Invalid("Begin address is not valid");
  if(_fram_size == 0)
    return Status::Invalid("FRAM size is not valid");
  std::shared_ptr<FramI2CFM24CLxx> fram(new FramI2CFM24CLxx(_hi2c, _begin_address, _fram_size));
  return Result<decltype(fram)>::OK(fram);
}

FramI2CFM24CLxx::FramI2CFM24CLxx(std::shared_ptr<I2C> hi2c, uint16_t begin_address, uint32_t fram_size)
: FramI2C(hi2c, 0xA0, begin_address, fram_size) {
}

Status FramI2CFM24CLxx::write(uint32_t address, const std::vector<uint8_t> &data) {
  // auto mayby_encoded_data = FRAM::encode_data(data);
  // if(!mayby_encoded_data.ok())
  // return mayby_encoded_data.status();
  STMEPIC_ASSING_OR_RETURN(encoded_data, FRAM::encode_data(data));
  // auto encoded_data = mayby_encoded_data.valueOrDie();
  if(encoded_data.size() > fram_size)
    return Status::CapacityError("Data is too big for the FRAM");
  uint32_t memory_address = begin_address + address;
  uint16_t dev_address    = device_address | ((memory_address >> 8) & 0x0E);
  return hi2c->write(dev_address, memory_address, encoded_data.data(), encoded_data.size());
}

Result<std::vector<uint8_t>> FramI2CFM24CLxx::read(uint32_t address) {
  std::vector<uint8_t> data;
  uint8_t data_size[2];
  uint32_t memory_address = begin_address + address;
  uint16_t dev_address    = device_address | ((memory_address >> 8) & 0x0E);
  STMEPIC_RETURN_ON_ERROR(hi2c->read(dev_address, memory_address + 7, data_size, 2));
  uint16_t size = (data_size[0] << 8) | data_size[1];
  data.resize(size + frame_size);
  STMEPIC_RETURN_ON_ERROR(hi2c->read(dev_address, memory_address, data.data(), size + frame_size));
  return FRAM::decode_data(data);
}
