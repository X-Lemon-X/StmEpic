#pragma once

#include "Timing.hpp"
#include "encoder.hpp"
#include "filter.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstddef>
#include "i2c.hpp"

#define ANGLE_MAX_DEFFERENCE 2.0f // 1 radian

/**
 * @file encoder_magnetic.hpp
 * @brief Base classes for typical magnetic encoders.
 */

/**
 * @defgroup Encoders
 * @{
 */


namespace stmepic::encoders {

/**
 * @brief Interface for I2C absolute magnetic encoders.
 *
 */
class EncoderAbsoluteMagnetic : public EncoderBase {
public:
  /// @brief reads the last calculated velocity
  /// @return the velocity in radians per second
  [[nodiscard]] float get_velocity() const override;

  /// @brief reads the last calculated torque
  /// @return the torque in Nm
  [[nodiscard]] float get_torque() const override;

  /// @brief gets the last calcualted angle with offset and reverse
  /// @return the angle in radians
  [[nodiscard]] float get_angle() const override;

  /// @brief gets the latest calculated absolute angle
  /// absoulte angle includes information how many times the encoder has rotated
  /// for example if the encoder has rotated 3 times the angle will be 6pi + the current angle
  /// @return the absoulte angle in radians
  [[nodiscard]] float get_absoulute_angle() const override;


  /// @brief set the ratio that will be multiplayed by value of the angle and velocity
  /// @param ratio the ratio that will be multiplayed by value of the angle and velocity
  void set_ratio(float ratio) override;

  /// @brief reads the angle from the encoder
  /// @return the angle in radians
  float read_angle();


  /// @brief sets the offset of the encoder
  /// @param offset the offset in radians
  void set_offset(float offset);

  /// @brief sets the reverse of the encoder
  /// @param reverse true if the encoder is reversed
  void set_reverse(bool reverse);

  /// @brief sets the begin roation dead zone correction angle
  /// The dead zone angle is used to correct initial value of the angle to be eather positive or negative.
  /// Since the same angle can be read as either positive or negative value for exmple -pi/2 or 3pi/2
  /// the dead zone angle is used to correct this by making the angle
  /// In the counter clockwise direction on the left side of the dead zone angle negative and on the right side positive
  /// @param begin_roation_dead_zone_correction_angle the angle in radians, can be set to 0 if dead zone correction angle shall not be used.
  void set_dead_zone_correction_angle(float begin_roation_dead_zone_correction_angle);

  bool device_ok() override;

  Result<bool> device_is_connected() override;

  Status device_get_status() override;

  Status device_reset() override;

  Status device_start() override;

  Status device_stop() override;


protected:
  /// @brief Init fucnion of the encoder
  EncoderAbsoluteMagnetic(std::shared_ptr<I2C> hi2c,
                          uint32_t resolution,
                          std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
                          std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);

  /// @brief Overide this fucniton with fucniton to read angle from  your encoder
  /// Reads the raw angle from the encoder
  /// @return the raw angle in uint32_t
  virtual Result<uint32_t> read_raw_angle() = 0;

  std::shared_ptr<I2C> hi2c;
  /// @brief Should be set to true if the encoder is connected
  bool encoder_connected;
  Status device_status;


private:
  std::shared_ptr<filters::FilterBase> filter_angle;
  std::shared_ptr<filters::FilterBase> filter_velocity;
  // traslate_reg_to_angle reg_to_angle_function;

  float last_time;
  float prev_angle;
  float current_angle;
  float current_velocity;
  float prev_angle_rad_raw;
  float prev_angle_velocity;
  float over_drive_angle;
  float absolute_angle;
  float ratio;

  float offset;
  float dead_zone_correction_angle;
  bool reverse;
  uint32_t resolution;

  /// @brief thsi should be run in a  constructor of child class to initiate the begining values using child class specific values
  void init() override;

  /// @brief Calucaltes velcoicty, and passes it thoroung a filter
  /// @param angle current angle
  /// @param current_time  current time
  /// @return returns the filtered velocity
  float calculate_velocity(float angle);


  /// @brief reads the angle from the encoder and converts it to radians
  /// applays the offset and the reverse
  float read_angle_rads();


  /// @brief handles the encoder updates data read from the encoder
  void handle();

  static void task_encoder_before(SimpleTask &handler, void *arg);
  static void task_encoder(SimpleTask &handler, void *arg);

  [[nodiscard]] Status do_device_task_start() override;
  [[nodiscard]] Status do_device_task_stop() override;
};


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
  Make(std::shared_ptr<I2C> hi2c,
       encoder_MT6701_addresses address                     = encoder_MT6701_addresses::MT6701_I2C_ADDRESS_1,
       std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
       std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);

  Result<uint32_t> read_raw_angle() override;

private:
  uint16_t address;
  EncoderAbsoluteMagneticMT6701(std::shared_ptr<I2C> hi2c,
                                uint16_t address,
                                uint32_t resolution,
                                std::shared_ptr<filters::FilterBase> filter_angle    = nullptr,
                                std::shared_ptr<filters::FilterBase> filter_velocity = nullptr);
};

} // namespace stmepic::encoders
