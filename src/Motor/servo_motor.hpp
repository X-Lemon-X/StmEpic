#pragma once

#include "Timing.hpp"
#include "motor.hpp"
#include "stmepic.hpp"
#include "encoder.hpp"


namespace stmepic::motor {

struct ServoMotorPWMSettings : public DeviceSettings {
  float min_pulse_width_us; // Minimum pulse width in micro seconds [us]
  float max_pulse_width_us; // Maximum pulse width in micro seconds [us]
  float pwm_frequency;      // Frequency of the PWM signal in Hz [Hz]
  float min_angle_rad;      // Minimum angle of the servo motor in radians [rad]
  float max_angle_rad;      // Maximum angle of the servo motor in radians [rad]
  uint8_t n_multiplayer; // By increasing this value, you DECREASE the resolution of the servo motor, But increase the prescaler of the timer,
  // which can be useful for low frequency uC with high frequency servo motors, because the clock is tool low to generate sufficient Frequency PWM.
};

/**
 * @class ServoMotorPWM
 * @brief Class for controlling the classic PWM controlled servo motors.
 *
 * This class provides a base interface for controlling servo motors.
 */
class ServoMotorPWM : public MotorBase {
public:
  /// @brief Constructor for the ServoMotorPWM class
  /// @param htim Timer handle for the PWM signal generation
  /// @param timer_channel Timer channel for the PWM signal
  /// @note The timer channel should be the same as the one used for the PWM signal
  ServoMotorPWM(TIM_HandleTypeDef &htim, unsigned int timer_channel);

  void set_velocity(float speed) override;

  void set_torque(float torque) override;

  void set_position(float position) override;

  void set_enable(bool enable) override;

  void set_gear_ratio(float gear_ratio) override;

  void set_reverse(bool reverse) override;

  void set_max_velocity(float max_velocity) override;

  void set_min_velocity(float min_velocity) override;

  /// @brief Get the current speed of the ServoMotor
  /// @return allways 0.0f, as servo motors are not speed controlled
  [[nodiscard]] float get_velocity() const override;

  /// @brief Get the current torque of the ServoMotor
  /// @return allways 0.0f, as servo motors are not torque controlled
  [[nodiscard]] float get_torque() const override;

  /// @brief Get the current position of the ServoMotor
  /// @return The position in radians, calculated from the pulse width and the settings
  [[nodiscard]] float get_position() const override;


  [[nodiscard]] float get_absolute_position() const override;

  [[nodiscard]] float get_gear_ratio() const override;

  bool device_ok() override;
  Result<bool> device_is_connected() override;
  Status device_get_status() override;
  Status device_reset() override;
  Status device_start() override;
  Status device_stop() override;
  Status device_set_settings(const DeviceSettings &settings) override;

private:
  TIM_HandleTypeDef &htim;
  unsigned int timer_channel;
  ServoMotorPWMSettings settings;

  float timer_values_to_freq(uint32_t prescaler, uint32_t counter_max, uint32_t timer_clock);

  float postion;

  float gear_ratio;
  bool reverse;
  bool enable;
  Status status;
  float count_per_rad;
  uint32_t min_pulse_width;
};

} // namespace stmepic::motor