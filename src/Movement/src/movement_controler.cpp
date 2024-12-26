
// #include "main.h"
#include "movement_controler.hpp"
#include "StmEpic/src/Motor/inc/motor.hpp"
#include "Timing.hpp"
#include "stm32f4xx_hal.h"
#include <cmath>

using namespace stmepic;
using namespace stmepic::movement;
using namespace stmepic::gpio;


MovementEquation::MovementEquation () {
}

MovementControler::MovementControler () {
  initialized                  = false;
  current_state.position       = 0;
  current_state.velocity       = 0;
  current_state.torque         = 0;
  target_state.position        = 0;
  target_state.velocity        = 0;
  target_state.torque          = 0;
  dont_override_limit_position = true;
}

MovementControler::~MovementControler () {
  if (initialized) {
    set_motor_state (MovementState{ 0, 0, motor->get_absolute_position () });
    motor->set_enable (false);
  }
}

void MovementControler::init (motor::MotorBase& _motor,
                              MovementControlMode _control_mode,
                              MovementEquation& _movement_equation) {
  motor                  = &_motor;
  movement_equation      = &_movement_equation;
  control_mode           = _control_mode;
  initialized            = true;
  current_state.position = motor->get_absolute_position ();
  current_state.velocity = motor->get_velocity ();
  current_state.torque   = motor->get_torque ();
  movement_equation->begin_state (current_state, Ticker::get_instance ().get_seconds ());
}

void MovementControler::handle () {
  if (!initialized)
    return;
  current_state.position = motor->get_absolute_position ();
  current_state.velocity = motor->get_velocity ();
  current_state.torque   = motor->get_torque ();

  auto state = movement_equation->calculate (current_state, target_state);

  state.velocity = overide_limit_abs (state.velocity, max_velocity);
  state.torque   = overide_limit_abs (state.torque, max_torque);

  if (dont_override_limit_position &&
      (current_state.position < min_position || current_state.position > max_position)) {
    state.velocity = 0;
    state.torque   = 0;
    state.position = overide_limit (current_state.position, max_position, min_position);
    limit_positon_achieved = true;
  } else {
    limit_positon_achieved = false;
  }

  motor->set_enable (enable);
  set_motor_state (state);
}

void MovementControler::set_motor_state (MovementState state) {
  switch (control_mode) {
  case MovementControlMode::VELOCITY: motor->set_velocity (state.velocity); break;
  case MovementControlMode::TORQUE: motor->set_torque (state.torque); break;
  case MovementControlMode::POSITION: motor->set_position (state.position); break;
  }
}

void MovementControler::set_torque (float torque) {
  target_state.torque = torque;
}

void MovementControler::set_velocity (float velocity) {
  target_state.velocity = velocity;
}

void MovementControler::set_enable (bool enable) {
  this->enable = enable;
}

void MovementControler::set_position (float position) {
  if (position < min_position)
    position = min_position;
  if (position > max_position)
    position = max_position;
  target_state.position = position;
}

void MovementControler::set_limit_position (float min_position, float max_position) {
  this->min_position = min_position;
  this->max_position = max_position;
}

void MovementControler::set_max_velocity (float max_velocity) {
  this->max_velocity = std::abs (max_velocity);
}

void MovementControler::set_max_torque (float max_torque) {
  this->max_torque = std::abs (max_torque);
}

float MovementControler::get_current_torque () const {
  return current_state.torque;
}

float MovementControler::get_current_position () const {
  return current_state.position;
}

float MovementControler::get_current_velocity () const {
  return current_state.velocity;
}

void MovementControler::override_limit_position (bool overide) {
  dont_override_limit_position = !overide;
}

bool MovementControler::get_limit_position_achieved () const {
  return limit_positon_achieved;
}

float MovementControler::overide_limit_abs (float value, float max, float min) {
  if (std::abs (value) > max)
    return value > 0 ? max : -max;
  if (std::abs (value) < min)
    return value > 0 ? min : -min;
  else
    return value;
}

float MovementControler::overide_limit (float value, float max, float min) {
  if (value > max)
    return max;
  if (value < min)
    return min;
  else
    return value;
}
