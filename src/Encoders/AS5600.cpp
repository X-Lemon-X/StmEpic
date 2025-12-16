#pragma once

#include "Timing.hpp"
#include "encoder.hpp"
#include "filter.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstddef>
#include "i2c.hpp"
#include "encoder_magnetic.hpp"
#include "AS5600.hpp"

using namespace stmepic::encoders;
using namespace stmepic;

//********************************************************************************************************************
// AS5600 Encoder

EncoderAbsoluteMagneticAS5600::EncoderAbsoluteMagneticAS5600(std::shared_ptr<I2cBase> hi2c,
                                                             uint16_t _address,
                                                             uint32_t resolution,
                                                             std::shared_ptr<filters::FilterBase> filter_angle,
                                                             std::shared_ptr<filters::FilterBase> filter_velocity)
: EncoderAbsoluteMagnetic(hi2c, resolution, filter_angle, filter_velocity), address(_address) {
}

Result<std::shared_ptr<EncoderAbsoluteMagneticAS5600>>
EncoderAbsoluteMagneticAS5600::Make(std::shared_ptr<I2cBase> hi2c,
                                    encoder_AS5600_addresses _address,
                                    std::shared_ptr<filters::FilterBase> filter_angle,
                                    std::shared_ptr<filters::FilterBase> filter_velocity) {
  if(hi2c == nullptr)
    return Status::Invalid("I2cBase is not nullptr");
  auto encoder = std::shared_ptr<EncoderAbsoluteMagneticAS5600>(
  new EncoderAbsoluteMagneticAS5600(hi2c, (uint16_t)_address, 4096, filter_angle, filter_velocity));
  return Result<decltype(encoder)>::OK(std::move(encoder));
}

Result<uint32_t> EncoderAbsoluteMagneticAS5600::read_raw_angle() {
  uint8_t data[2];
  auto status       = hi2c->read(address, 0x0C, data, 2);
  encoder_connected = status.ok();
  device_status     = status;
  STMEPIC_RETURN_ON_ERROR(status);
  uint32_t reg = (uint32_t)(data[0] & 0x0F) << 8;
  reg |= (uint32_t)(data[1]);
  return Result<uint32_t>::OK(std::move(reg));
}
