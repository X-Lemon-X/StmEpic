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
 * @file MT6701.hpp
 * @brief Class for controlling the MT6701 magnetic encoder.
 */

/**
 * @defgroup Encoders
 * @{
 */

/**
 * @defgroup MT6701c MT6701
 * @brief Class for controlling the MT6701 magnetic encoder.
 * @{
 */


namespace stmepic::encoders {

//********************************************************************************************************************
// Magnatek MT6701 Encoder

/// @brief Address of the MT6701 encoders second one can be programed
enum class encoder_MT6701_addresses : uint16_t { MT6701_I2C_ADDRESS_1 = 0x06, MT6701_I2C_ADDRESS_2 = 0x46 };


/**
 * @brief Class for MT6701 absoulute magnetic encoder from MagnTek
 * The encoder is a 14 bit magentic absoulute encoder
 */
class EncoderAbsoluteMagneticMT6701 : public EncoderAbsoluteMagnetic {
public:
  static Result<std::shared_ptr<EncoderAbsoluteMagneticMT6701>>
  Make(std::shared_ptr<I2cBase> hi2c,
       encoder_MT6701_addresses address                     = encoder_MT6701_addresses::MT6701_I2C_ADDRESS_1,
       std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
       std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);

  Result<uint32_t> read_raw_angle() override;

private:
  uint16_t address;
  EncoderAbsoluteMagneticMT6701(std::shared_ptr<I2cBase> hi2c,
                                uint16_t address,
                                uint32_t resolution,
                                std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
                                std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);
};


} // namespace stmepic::encoders