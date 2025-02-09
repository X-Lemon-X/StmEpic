
// #include "main.h"
#include "movement_controler.hpp"
#include "Timing.hpp"
#include "motor.hpp"
#include <cmath>

using namespace stmepic;
using namespace stmepic::movement;


MovementEquation::MovementEquation() {
}

MovementControler::MovementControler()
: initialized(false), current_state({ 0, 0, 0 }), target_state({ 0, 0, 0 }), dont_override_limit_position(true),
  motor(nullptr), movement_equation(nullptr), control_mode(MovementControlMode::POSITION), enable(false),
  limit_positon_achieved(false), max_position(0), min_position(0), max_torque(0), max_velocity(0) {
  task.task_init(handle, this, 1, nullptr, 300, tskIDLE_PRIORITY + 2, "MovementControler");
};

MovementControler::~MovementControler() {
  if(initialized) {
    set_motor_state(MovementState{ 0, 0, motor->get_absolute_position() });
    motor->set_enable(false);
  }
  (void)task.task_stop();
}

void MovementControler::init(motor::MotorBase &_motor, MovementControlMode _control_mode, MovementEquation &_movement_equation) {
  motor                  = &_motor;
  movement_equation      = &_movement_equation;
  control_mode           = _control_mode;
  initialized            = true;
  current_state.position = motor->get_absolute_position();
  current_state.velocity = motor->get_velocity();
  current_state.torque   = motor->get_torque();
  movement_equation->begin_state(current_state, Ticker::get_instance().get_seconds());
  (void)task.task_stop();
  task.task_run();
}

void MovementControler::handle(SimpleTask &task, void *args) {
  auto mc = static_cast<MovementControler *>(args);
  if(!mc->initialized)
    return;
  mc->current_state.position = mc->motor->get_absolute_position();
  mc->current_state.velocity = mc->motor->get_velocity();
  mc->current_state.torque   = mc->motor->get_torque();

  auto state = mc->movement_equation->calculate(mc->current_state, mc->target_state);

  state.velocity = overide_limit_abs(state.velocity, mc->max_velocity);
  state.torque   = overide_limit_abs(state.torque, mc->max_torque);

  if(mc->dont_override_limit_position &&
     (mc->current_state.position < mc->min_position || mc->current_state.position > mc->max_position)) {
    state.velocity = 0;
    state.torque   = 0;
    state.position = overide_limit(mc->current_state.position, mc->max_position, mc->min_position);
    mc->limit_positon_achieved = true;
  } else {
    mc->limit_positon_achieved = false;
  }

  mc->motor->set_enable(mc->enable);
  mc->set_motor_state(state);
}

void MovementControler::set_motor_state(MovementState state) {
  switch(control_mode) {
  case MovementControlMode::VELOCITY: motor->set_velocity(state.velocity); break;
  case MovementControlMode::TORQUE: motor->set_torque(state.torque); break;
  case MovementControlMode::POSITION: motor->set_position(state.position); break;
  }
}

void MovementControler::set_torque(float torque) {
  target_state.torque = torque;
}

void MovementControler::set_velocity(float velocity) {
  target_state.velocity = velocity;
}

void MovementControler::set_enable(bool enable) {
  this->enable = enable;
}

void MovementControler::set_position(float position) {
  if(position < min_position)
    position = min_position;
  if(position > max_position)
    position = max_position;
  target_state.position = position;
}

void MovementControler::set_limit_position(float min_position, float max_position) {
  this->min_position = min_position;
  this->max_position = max_position;
}

void MovementControler::set_max_velocity(float max_velocity) {
  this->max_velocity = std::abs(max_velocity);
}

void MovementControler::set_max_torque(float max_torque) {
  this->max_torque = std::abs(max_torque);
}

float MovementControler::get_current_torque() const {
  return current_state.torque;
}

float MovementControler::get_current_position() const {
  return current_state.position;
}

float MovementControler::get_current_velocity() const {
  return current_state.velocity;
}

void MovementControler::override_limit_position(bool overide) {
  dont_override_limit_position = !overide;
}

bool MovementControler::get_limit_position_achieved() const {
  return limit_positon_achieved;
}

float MovementControler::overide_limit_abs(float value, float max, float min) {
  if(std::abs(value) > max)
    return value > 0 ? max : -max;
  if(std::abs(value) < min)
    return value > 0 ? min : -min;
  else
    return value;
}

float MovementControler::overide_limit(float value, float max, float min) {
  if(value > max)
    return max;
  if(value < min)
    return min;
  else
    return value;
}
