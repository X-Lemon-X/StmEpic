

#ifndef STEPER_MOTOR_HPP
#define STEPER_MOTOR_HPP

#include "stm32f4xx_hal.h"
#include "Timing.hpp"
#include "pin.hpp"


namespace STEPER_MOTOR
{
class SteperMotor{
private:
  float radians_to_frequency;
  float frequency;
  float angle;
  TIM_HandleTypeDef &htim;
  unsigned int timer_channel;
  const GpioPin &direction_pin;
  const GpioPin &enable_pin;

  float steps_per_revolution;
  float gear_ratio;
  float max_velocity;
  float min_velocity;
  bool reverse;
  bool enable_reversed;
public:


  SteperMotor(TIM_HandleTypeDef &htim,unsigned int timer_channel,const GpioPin &direction_pin, const GpioPin &enable_pin);
  
  /// @brief Initialize the SteperMotor, calcualtes all necessary stuff to avoid calcualting it over again
  /// after the initialization
  void init();

  /// @brief Set the current speed of the SteperMotor
  /// @param speed The speed in radians per second, can be negative or positive to change the direction
  void set_velocity(float speed);

  /// @brief enable or disable the SteperMotor, can be used as a break
  /// @param enable True to enable the SteperMotor, false to disable it
  void set_enable(bool enable);

  /// @brief Set the steps per revolution of the SteperMotor
  void set_steps_per_revolution(float steps_per_revolution);

  /// @brief Set the gear ratio of the SteperMotor
  void set_gear_ratio(float gear_ratio);

  /// @brief Set the max velocity of the SteperMotor
  void set_max_velocity(float max_velocity);

  /// @brief Set the min velocity of the SteperMotor
  void set_min_velocity(float min_velocity);

  /// @brief Set the reverse of the SteperMotor
  void set_reverse(bool reverse);

  /// @brief Sometimes enable pin in negated so you can enable the SteperMotor by setting the enable pin to low instead of high
  /// @param enable_reversed True if the enable pin is negated, false if not
  void set_enable_reversed(bool enable_reversed);

  /// @brief Get the gear ratio of the SteperMotor
  float get_gear_ratio() const;
};
}


#endif // STEPER_MOTOR_HPP