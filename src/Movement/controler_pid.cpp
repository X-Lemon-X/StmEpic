
#include "controler_pid.hpp"
#include "Timing.hpp"
#include <cmath>

using namespace stmepic;
using namespace stmepic::movement;


PIDController::PIDController() : MovementEquation() {
}

void PIDController::set_velocity_pid_config(const stmepic::controller::PidConfig &cfg) {
  velocity_pid.setConfig(cfg);
}

void PIDController::set_position_pid_config(const stmepic::controller::PidConfig &cfg) {
  position_pid.setConfig(cfg);
}

void PIDController::set_torque_pid_config(const stmepic::controller::PidConfig &cfg) {
  torque_pid.setConfig(cfg);
}


MovementState PIDController::calculate(MovementState current_state, MovementState target_state) {
  const float current_time = Ticker::get_instance().get_seconds();
  float dt                 = current_time - previous_time;
  if(dt <= 0)
    dt = 1e-6f;
  previous_time = current_time;

  MovementState out_state = current_state;
  switch(target_state.mode) {
  case MovementControlMode::POSITION:
    // position control
    position_pid.setSetpoint(target_state.position);
    out_state.velocity = position_pid.getOutput(current_state.position, dt);
    velocity_pid.setSetpoint(out_state.velocity);
    out_state.torque = velocity_pid.getOutput(current_state.velocity, dt);
    break;
  case MovementControlMode::VELOCITY:
    // velocity control
    velocity_pid.setSetpoint(target_state.velocity);
    out_state.torque = velocity_pid.getOutput(current_state.velocity, dt);
    break;
  case MovementControlMode::TORQUE:
    // torque control
    out_state.torque = target_state.torque;
    break;
  default:
    // handle unknown mode if needed
    break;
  }
  return out_state;
}

void PIDController::begin_state(MovementState current_state, float current_time) {
  previous_state = current_state;
  previous_time  = current_time;
}