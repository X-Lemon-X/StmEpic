
#pragma once

#include "Timing.hpp"
#include "device.hpp"
#include "encoder.hpp"
#include "gpio.hpp"

/**
 * @file motor.hpp
 * @brief Base interface class for all Actuators.
 *  Base interface for all Actuators, provides basic functionalities for controlling the movement of the actuator.
 */

/**
 * @defgroup Motor
 * @brief Motor control interface.
 * All kind of motors with different control methods combined into single interface.
 * Control BLDC motors, Stepper motors, DC motors, etc.
 * @{
 */


namespace stmepic::motor {

/**
 * @class MotorBase
 * @brief Base interface class for all Actuators.
 *
 * Base interface for all Actuators, provides basic functionalities for controlling the movement of the actuator.
 *
 */
class MotorBase : public DeviceBase {
public:
  MotorBase() = default;

  /// @brief Initialize the Motor, calcualtes all necessary stuff to a void calcualting it
  /// over again after the initialization
  virtual void init() = 0;


  /// @brief Get the current speed of the Motor
  /// @return The speed in radians per second
  [[nodiscard]] virtual float get_velocity() const = 0;

  /// @brief Get the current torque of the Motor
  /// @return The torque in Nm
  [[nodiscard]] virtual float get_torque() const = 0;

  /// @brief Get the current position of the Motor
  /// @return The position in radians
  [[nodiscard]] virtual float get_position() const = 0;

  /// @brief Get the absolute position of the Motor
  /// @return The position in radians
  [[nodiscard]] virtual float get_absolute_position() const = 0;

  /// @brief Get the gear ratio of the Motor
  [[nodiscard]] virtual float get_gear_ratio() const = 0;

  /// @brief Set the current speed of the Motor
  /// @param speed The speed in radians per second, can be negative or positive to change the direction
  virtual void set_velocity(float speed) = 0;

  /// @brief Set the current torque of the Motor
  /// @param torque The torque in Nm, can be negative or positive to change the direction
  virtual void set_torque(float torque) = 0;

  /// @brief Set the current position of the Motor
  /// @param position The position in radians
  virtual void set_position(float position) = 0;


  /// @brief enable or disable the Motor, can be used as a break
  /// @param enable True to enable the Motor, false to disable it
  virtual void set_enable(bool enable) = 0;

  /// @brief Set the gear ratio of the Motor
  virtual void set_gear_ratio(float gear_ratio) = 0;

  /// @brief Set the max velocity of the Motor
  virtual void set_max_velocity(float max_velocity) = 0;

  /// @brief Set the min velocity of the Motor
  virtual void set_min_velocity(float min_velocity) = 0;

  /// @brief Set the reverse of the Motor
  virtual void set_reverse(bool reverse) = 0;
};


/**
 * @class MotorClosedLoop
 * @brief Class for controlling the Motor with closed loop control if the motor don't support its own closed loop.
 *
 * This class allows to combine a Motor with 1, 2 or 3 encoders to create a closed loop control for the Actuator.
 *
 */
class MotorClosedLoop : public MotorBase {
public:
  /// @brief Constructor for the SteperMotorClosedLoop class taht takes a SteperMotor, and 0 or 1 to 2 encoders
  /// @param steper_motor The SteperMotor that will be controlled
  /// @param encoder_pos The encoder that will be used to get position of the SteperMotor
  /// @param encoder_vel The encoder that will be used to get velocity of the SteperMotor
  /// The same encoder_pos and encoder_vel can be passed if the SteperMotor has only one encoder
  MotorClosedLoop(MotorBase &_motor,
                  std::shared_ptr<encoders::EncoderBase> _encoder_pos,
                  std::shared_ptr<encoders::EncoderBase> _encoder_vel,
                  std::shared_ptr<encoders::EncoderBase> _encoder_torque);

  void init() override;

  [[nodiscard]] float get_velocity() const override;

  [[nodiscard]] float get_torque() const override;

  [[nodiscard]] float get_position() const override;

  [[nodiscard]] float get_absolute_position() const override;

  [[nodiscard]] float get_gear_ratio() const override;

  void set_velocity(float speed) override;

  void set_torque(float torque) override;

  void set_position(float position) override;

  void set_enable(bool enable) override;

  void set_gear_ratio(float gear_ratio) override;

  void set_max_velocity(float max_velocity) override;

  void set_min_velocity(float min_velocity) override;

  void set_reverse(bool reverse) override;

  Result<bool> device_is_connected() override;

  bool device_ok() override;

  Status device_get_status() override;

  Status device_reset() override;

  Status device_start() override;

  Status device_stop() override;

private:
  MotorBase &motor;
  std::shared_ptr<encoders::EncoderBase> encoder_pos;
  std::shared_ptr<encoders::EncoderBase> encoder_vel;
  std::shared_ptr<encoders::EncoderBase> encoder_torque;
};

} // namespace stmepic::motor
