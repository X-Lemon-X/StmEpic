
#include "controler_pid.hpp"
#include "Timing.hpp"
#include <cmath>

using namespace stmepic;
using namespace stmepic::movement;


PIDControler::PIDControler() : MovementEquation() {
  Kp             = 0;
  Kd             = 0;
  Ki             = 0;
  previous_state = MovementState{ 0, 0, 0 };
  previous_time  = 0;
}

MovementState PIDControler::calculate(MovementState current_state, MovementState target_state) {
  const float current_time = Ticker::get_instance().get_seconds();
  // const float state_velocity = (previous_position-current_position) / (current_time - previous_time);
  float dt      = current_time - previous_time;
  previous_time = current_time;
  // return Kp * (target_position - current_position) + Kd * (target_velocity -
  // state_velocity); float error = target_velocity - current_velocity; add inertia to the
  // system float error_position= target_position - current_position; float error_velocity
  // = target_velocity - current_velocity; float velocity =  (Kp * error_position) + (Kd *
  // error_position/dt); float error_velocity = target_velocity - current_velocity; float
  // error_i = ; float velocity =  (Kp * previous_velocity) + (Kd * target_velocity);
  float next_position     = (Kd * current_state.position) + (target_state.position * Ki);
  float velocity          = (next_position - previous_state.position) / dt;
  previous_state.position = next_position;
  previous_state.velocity = velocity;
  return previous_state;
}


void PIDControler::begin_state(MovementState current_state, float current_time) {
  previous_state = current_state;
  previous_time  = current_time;
}