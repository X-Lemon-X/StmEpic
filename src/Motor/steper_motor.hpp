#pragma once

#include "Timing.hpp"
#include "encoder.hpp"
#include "motor.hpp"
#include "movement_controler.hpp"
#include "gpio.hpp"
#include "stmepic.hpp"

/**
 * @file steper_motor.hpp
 * @brief Class definition for contorling steper motors.
 *
 */

/**
 * @defgroup Motor
 * @{
 */

namespace stmepic::motor {

/**
 * @class SteperMotorStepDir
 * @brief Class for controlling the SteperMotor with step and direction control.
 *
 * This class allows to control the SteperMotor with simple control interface like step,
 * direction and pulse Mostly used with steper motor drivers like A4988, DRV8825, etc.
 *
 */
class SteperMotorStepDir : public MotorBase {
public:
  /// @brief Constructor for the SteperMotorStepDir class
  SteperMotorStepDir(TIM_HandleTypeDef &htim, unsigned int timer_channel, GpioPin &direction_pin, GpioPin &enable_pin);

  /// @brief Initialize the SteperMotor, calculates all necessary stuff to avoid
  /// calculating it over again after the initialization
  void init() override;

  /// @brief Set the current speed of the SteperMotor
  /// @param speed The speed in radians per second, can be negative or positive to change the direction
  void set_velocity(float speed) override;

  /// @brief Set the current torque of the SteperMotor
  /// @param torque The torque in Nm, can be negative or positive to change the direction
  void set_torque(float torque) override;

  /// @brief Set the current position of the SteperMotor
  /// @param position The position in radians
  void set_position(float position) override;

  /// @brief enable or disable the SteperMotor, can be used as a break
  /// @param enable True to enable the SteperMotor, false to disable it
  void set_enable(bool enable) override;

  /// @brief Set the steps per revolution of the SteperMotor
  void set_steps_per_revolution(float steps_per_revolution);

  /// @brief Set the gear ratio of the SteperMotor
  void set_gear_ratio(float gear_ratio) override;

  /// @brief Set the max velocity of the SteperMotor
  void set_max_velocity(float max_velocity) override;

  /// @brief Set the min velocity of the SteperMotor
  void set_min_velocity(float min_velocity) override;

  /// @brief Set the reverse of the SteperMotor
  void set_reverse(bool reverse) override;

  /// @brief Sometimes enable pin in negated so you can enable the SteperMotor by setting the enable pin to low instead of high
  /// @param enable_reversed True if the enable pin is negated, false if not
  void set_reversed_enable_pin(bool enable_reversed);

  /// @brief Set the prescaler of the timer used by the SteperMotor
  /// if not set the prescaler will be the same as the timer
  void set_prescaler(uint32_t prescaler);

  /// @brief Get the gear ratio of the SteperMotor
  float get_gear_ratio() const override;

  /// @brief Get the current speed of the SteperMotor
  /// @return The speed in radians per second
  float get_velocity() const override;

  /// @brief Get the current torque of the SteperMotor
  /// @return The torque in Nm
  float get_torque() const override;

  /// @brief Get the current position of the SteperMotor
  /// @return The position in radians
  float get_position() const override;

  /// @brief Get the absolute position of the SteperMotor
  /// @return The position in radians
  float get_absolute_position() const override;

  bool device_ok() override;

  Result<bool> device_is_connected() override;

  Status device_get_status() override;

  Status device_reset() override;

  Status device_start() override;

  Status device_stop() override;

private:
  float radians_to_frequency;
  float frequency;
  float angle;
  TIM_HandleTypeDef &htim;
  unsigned int timer_channel;
  GpioPin &direction_pin;
  GpioPin &enable_pin;

  float steps_per_revolution;
  float gear_ratio;
  float max_velocity;
  float min_velocity;
  bool reverse;
  bool enable_reversed;

  movement::MovementState current_state;
};

} // namespace stmepic::motor
