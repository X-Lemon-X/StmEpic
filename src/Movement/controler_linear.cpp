#include "controler_linear.hpp"
#include "Timing.hpp"
#include <cmath>

using namespace stmepic;
using namespace stmepic::movement;


BasicLinearPosControler::BasicLinearPosControler() : MovementEquation() {
  max_acceleration     = 0;
  target_pos_max_error = 0;
  previous_state       = MovementState{ 0, 0, 0 };
  current_state        = MovementState{ 0, 0, 0 };
  previous_time        = 0;
}

float BasicLinearPosControler::get_sign(float value) {
  return value > 0.0f ? 1.0f : -1.0f;
}

MovementState BasicLinearPosControler::calculate(MovementState current_state, MovementState target_state) {
  float current_time = Ticker::get_instance().get_seconds();
  float dt           = current_time - previous_time;
  previous_time      = current_time;

  current_state.velocity = previous_state.velocity; // delete this line after velocity is implemented in the movement controler
  target_state.velocity = std::abs(target_state.velocity);

  float error_position          = target_state.position - current_state.position;
  float deacceleration_time     = std::abs(current_state.velocity / max_acceleration);
  float deacceleration_distance = std::abs((current_state.velocity * deacceleration_time) -
                                           (0.5 * max_acceleration * deacceleration_time * deacceleration_time));

  if(std::abs(error_position) > deacceleration_distance)
    current_state.velocity += get_sign(error_position) * max_acceleration * dt;
  else
    current_state.velocity -= get_sign(current_state.velocity) * max_acceleration * dt;

  if(current_state.velocity > target_state.velocity)
    current_state.velocity = target_state.velocity;
  else if(current_state.velocity < -target_state.velocity)
    current_state.velocity = -target_state.velocity;

  // if(std::abs(error_position) < target_pos_max_error) current_velocity = 0;

  previous_state.velocity = current_state.velocity;
  return current_state;
}

void BasicLinearPosControler::begin_state(MovementState current_state, float current_time) {
  previous_time  = current_time;
  current_state  = current_state;
  previous_state = current_state;
}

void BasicLinearPosControler::set_max_acceleration(float max_acceleration) {
  this->max_acceleration = max_acceleration;
};

void BasicLinearPosControler::set_target_pos_max_error(float target_pos_max_error) {
  this->target_pos_max_error = target_pos_max_error;
};