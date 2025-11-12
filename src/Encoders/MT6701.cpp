#include "Timing.hpp"
#include "encoder.hpp"
#include "filter.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstddef>
#include "i2c.hpp"
#include "encoder_magnetic.hpp"
#include "encoder.hpp"
#include "MT6701.hpp"

using namespace stmepic::encoders;
using namespace stmepic;


//********************************************************************************************************************
// Magnatek MT6701 Encoder

EncoderAbsoluteMagneticMT6701::EncoderAbsoluteMagneticMT6701(std::shared_ptr<I2cBase> hi2c,
                                                             uint16_t _address,
                                                             uint32_t resolution,
                                                             std::shared_ptr<filters::FilterBase> filter_angle,
                                                             std::shared_ptr<filters::FilterBase> filter_velocity)
: EncoderAbsoluteMagnetic(hi2c, resolution, filter_angle, filter_velocity), address(_address) {
}
Result<std::shared_ptr<EncoderAbsoluteMagneticMT6701>>
EncoderAbsoluteMagneticMT6701::Make(std::shared_ptr<I2cBase> hi2c,
                                    encoder_MT6701_addresses address,
                                    std::shared_ptr<filters::FilterBase> filter_angle,
                                    std::shared_ptr<filters::FilterBase> filter_velocity) {
  if(hi2c == nullptr)
    return Status::Invalid("I2cBase is not initialized");
  auto encoder = std::shared_ptr<EncoderAbsoluteMagneticMT6701>(
  new EncoderAbsoluteMagneticMT6701(hi2c, (uint16_t)address, 16384, filter_angle, filter_velocity));
  return Result<decltype(encoder)>::OK(std::move(encoder));
}

Result<uint32_t> EncoderAbsoluteMagneticMT6701::read_raw_angle() {
  uint8_t data[2];
  auto status       = hi2c->read(address, 0x03, data, 2);
  encoder_connected = status.ok() || status.status_code() != StatusCode::TimeOut;
  device_status     = status;
  STMEPIC_RETURN_ON_ERROR(status);
  uint32_t reg = (uint32_t)data[0] << 6;
  reg |= (uint32_t)(data[1] & 0xfc) >> 2;
  return Result<uint32_t>::OK(std::move(reg));
}