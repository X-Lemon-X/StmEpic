
#include "steper_motor.hpp"
#include "stm32f4xx_hal_gpio.h"
#include <cmath>

using namespace STEPER_MOTOR;

#define PIM2 6.28318530717958647692f

template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

SteperMotor::SteperMotor(TIM_HandleTypeDef &_htim,unsigned int _timer_channel,const GpioPin &_direction_pin,const GpioPin &_enable_pin):
htim(_htim),
direction_pin(_direction_pin),
timer_channel(_timer_channel),
enable_pin(_enable_pin){
  this->radians_to_frequency = 0;
  this->steps_per_revolution = 400;
  this->gear_ratio = 1;
  this->max_velocity = 0;
  this->min_velocity = 0;
  this->reverse = false;
  this->init();
}


void SteperMotor::init(){
  auto core_freq = (float)HAL_RCC_GetHCLKFreq();
  auto prescaler = (float)htim.Instance->PSC;

  // equaions to get frequency that will give us desired velocity
  // base frequency
  // frequency = velocity * steps_pre_revolutions * gear_ratio
  this->radians_to_frequency = core_freq / prescaler / ((this->steps_per_revolution * this->gear_ratio) / PIM2  );

}

void SteperMotor::set_velocity(float velocity){

  if (std::abs(velocity) > this->max_velocity)
    velocity = sgn(velocity) * this->max_velocity;
  else if (std::abs(velocity) < this->min_velocity){
    htim.Instance->CCR1 = 0;
    return;
  }

  // reverse the direction if the velocity is negative
  // if direction is reversed, the motor will move in the opposite direction
  bool direction = !reverse;
  if (velocity > 0)
    WRITE_GPIO(direction_pin,direction);
  else {
    WRITE_GPIO(direction_pin,!direction);
    velocity = -velocity;
  }

  if(velocity == 0){
    htim.Instance->CCR1 = 0;
    return;
  }
  uint32_t counter = (uint32_t)(this->radians_to_frequency / velocity);
  htim.Instance->ARR = counter;
  htim.Instance->CCR1 = counter/2;
}


void SteperMotor::set_enable(bool enable){
  uint8_t enable_pin_state = enable && !this->enable_reversed;
  WRITE_GPIO(enable_pin, enable_pin_state);
}


void SteperMotor::set_steps_per_revolution(float steps_per_revolution){
  this->steps_per_revolution = steps_per_revolution;
}

void SteperMotor::set_gear_ratio(float gear_ratio) {
  this->gear_ratio = gear_ratio;
}

void SteperMotor::set_max_velocity(float max_velocity){
  this->max_velocity = max_velocity;
}

void SteperMotor::set_min_velocity(float min_velocity){
  this->min_velocity = min_velocity;
}

void SteperMotor::set_reverse(bool reverse){
  this->reverse = reverse;
}

void SteperMotor::set_enable_reversed(bool enable_reversed){
  this->enable_reversed = enable_reversed;
}

float SteperMotor::get_gear_ratio() const { 
  return gear_ratio;
}