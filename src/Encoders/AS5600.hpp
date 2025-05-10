#pragma once

#include "Timing.hpp"
#include "encoder.hpp"
#include "filter.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstddef>
#include "i2c.hpp"
#include "encoder_magnetic.hpp"

/**
 * @file AS5600.hpp
 * @brief Class for controlling the AS5600 magnetic encoder.
 *
 */

/**
 * @defgroup Encoders
 * @{
 */

/**
 * @defgroup AS5600_encoders AS5600
 * @brief Class for controlling the AS5600 magnetic encoder.
 *@{
 */

namespace stmepic::encoders {
//********************************************************************************************************************
// AS5600 Encoder

/// @brief Address of the AS5600 encoders
enum class encoder_AS5600_addresses : uint16_t { AS5600_I2C_ADDRESS_1 = 0x36 };

/**
 * @brief Class for AS5600 absoulute magnetic encoder from ams AG
 * The encoder is a 12 bit magentic absoulute encoder requires amgnet to work with about 0.5-2 mm distance
 */
class EncoderAbsoluteMagneticAS5600 : public EncoderAbsoluteMagnetic {
public:
  static Result<std::shared_ptr<EncoderAbsoluteMagneticAS5600>>
  Make(std::shared_ptr<I2C> hi2c,
       encoder_AS5600_addresses address                     = encoder_AS5600_addresses::AS5600_I2C_ADDRESS_1,
       std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
       std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);
  Result<uint32_t> read_raw_angle() override;

private:
  uint16_t address;
  EncoderAbsoluteMagneticAS5600(std::shared_ptr<I2C> hi2c,
                                uint16_t address,
                                uint32_t resolution,
                                std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
                                std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);
};


} // namespace stmepic::encoders