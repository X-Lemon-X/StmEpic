
#include "steper_motor.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cmath>
#include <cstdint>


using namespace stmepic::motor;
using namespace stmepic;

#define PIM2 6.28318530717958647692f


template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

SteperMotorStepDir::SteperMotorStepDir(TIM_HandleTypeDef &_htim, unsigned int _timer_channel, GpioPin &_direction_pin, GpioPin &_enable_pin)
: htim(_htim), direction_pin(_direction_pin), timer_channel(_timer_channel), enable_pin(_enable_pin) {
  this->radians_to_frequency = 0;
  this->steps_per_revolution = 400;
  this->gear_ratio           = 1;
  this->max_velocity         = 0;
  this->min_velocity         = 0;
  this->reverse              = false;
  this->enable_reversed      = false;
  this->current_state        = movement::MovementState{ 0, 0, 0 };
  this->init();
}


void SteperMotorStepDir::init() {
  auto core_freq = (float)HAL_RCC_GetHCLKFreq();
  auto prescaler = (float)htim.Instance->PSC;

  // equaions to get frequency that will give us desired velocity
  // base frequency
  // frequency = velocity * steps_pre_revolutions * gear_ratio
  this->radians_to_frequency = core_freq / prescaler / ((this->steps_per_revolution * this->gear_ratio) / PIM2);
}

void SteperMotorStepDir::set_velocity(float velocity) {


  if(std::abs(velocity) > this->max_velocity)
    velocity = sgn(velocity) * this->max_velocity;
  else if(std::abs(velocity) < this->min_velocity) {
    htim.Instance->CCR1 = 0;
    return;
  }

  // reverse the direction if the velocity is negative
  // if direction is reversed, the motor will move in the opposite direction
  bool direction = !reverse;
  if(velocity > 0)
    direction_pin.write(direction);
  else {
    direction_pin.write(!direction);
    velocity = -velocity;
  }

  current_state.velocity = velocity;
  if(velocity == 0) {
    htim.Instance->CCR1 = 0;
    return;
  }

  uint32_t counter    = (uint32_t)(this->radians_to_frequency / velocity);
  htim.Instance->ARR  = counter;
  htim.Instance->CCR1 = counter / 2;
}

void SteperMotorStepDir::set_torque(float torque) {
  // not implemented
  current_state.torque = torque;
}

void SteperMotorStepDir::set_position(float position) {
  // not implemented
  current_state.position = position;
}

float SteperMotorStepDir::get_velocity() const {
  return current_state.velocity;
}

float SteperMotorStepDir::get_torque() const {
  // not implemented
  return current_state.torque;
}

float SteperMotorStepDir::get_position() const {
  // not implemented
  return current_state.position;
}

float SteperMotorStepDir::get_absolute_position() const {
  // not implemented
  return current_state.position;
}

void SteperMotorStepDir::set_enable(bool enable) {
  uint8_t enable_pin_state = enable ^ enable_reversed;
  enable_pin.write(enable_pin_state);
}


void SteperMotorStepDir::set_steps_per_revolution(float steps_per_revolution) {
  this->steps_per_revolution = steps_per_revolution;
}

void SteperMotorStepDir::set_gear_ratio(float gear_ratio) {
  this->gear_ratio = gear_ratio;
}

void SteperMotorStepDir::set_max_velocity(float max_velocity) {
  this->max_velocity = max_velocity;
}

void SteperMotorStepDir::set_min_velocity(float min_velocity) {
  this->min_velocity = min_velocity;
}

void SteperMotorStepDir::set_reverse(bool reverse) {
  this->reverse = reverse;
}

void SteperMotorStepDir::set_reversed_enable_pin(bool enable_reversed) {
  this->enable_reversed = enable_reversed;
}

void SteperMotorStepDir::set_prescaler(uint32_t prescaler) {
  htim.Instance->PSC = prescaler;
}

float SteperMotorStepDir::get_gear_ratio() const {
  return gear_ratio;
}

bool SteperMotorStepDir::device_ok() {
  return true;
}

Result<bool> SteperMotorStepDir::device_is_connected() {
  return Result<bool>::OK(true);
}

Status SteperMotorStepDir::device_get_status() {
  return Status::OK();
}

Status SteperMotorStepDir::device_reset() {
  return Status::OK();
}

Status SteperMotorStepDir::device_start() {
  return Status::OK();
}

Status SteperMotorStepDir::device_stop() {
  return Status::OK();
}
