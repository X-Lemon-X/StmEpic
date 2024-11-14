
#pragma once

#include "Timing.hpp"
#include "encoder.hpp"
#include "pin.hpp"



namespace stmepic
{
class MotorBase{
public:
  MotorBase() = default;
  
  /// @brief Initialize the Motor, calcualtes all necessary stuff to a void calcualting it over again
  /// after the initialization
  virtual void init() = 0;


  /// @brief Get the current speed of the Motor
  /// @return The speed in radians per second
  virtual float get_velocity() const = 0;

  /// @brief Get the current torque of the Motor
  /// @return The torque in Nm
  virtual float get_torque() const = 0;

  /// @brief Get the current position of the Motor
  /// @return The position in radians
  virtual float get_position() const = 0;

  /// @brief Get the absolute position of the Motor
  /// @return The position in radians
  virtual float get_absolute_position() const = 0;


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

  /// @brief Sometimes enable pin in negated so you can enable the Motor by setting the enable pin to low instead of high
  /// @param enable_reversed True if the enable pin is negated, false if not
  virtual void set_enable_reversed(bool enable_reversed) = 0;

  /// @brief Set the prescaler of the timer used by the Motor
  /// if not set the prescaler will be the same as the timer
  virtual void set_prescaler(uint32_t prescaler) = 0;

  /// @brief Get the gear ratio of the Motor
  virtual float get_gear_ratio() const = 0;
};

}

