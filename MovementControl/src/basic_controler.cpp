#include "basic_controler.hpp"
#include <cmath>

using namespace CONTROLER;


BasicControler::BasicControler(TIMING::Ticker &ticker): MOVEMENT_CONTROLER::MovementEquation(ticker){
  max_acceleration = 0;
  target_pos_max_error = 0;
  previous_velocity = 0;
  previous_position = 0;
  previous_time = 0;
}

float BasicControler::get_sign(float value){
  return value > 0.0f ? 1.0f : -1.0f;
}

float BasicControler::calculate(float current_position, float target_position, float current_velocity, float target_velocity){  
  const float current_time = ticker.get_seconds();
  float dt = current_time - previous_time;
  previous_time = current_time;

  current_velocity = previous_velocity; // delete this line after velocity is implemented in the movement controler

  float error_position= target_position - current_position;
  float deacceleration_time = std::abs(current_velocity / max_acceleration);  
  float deacceleration_distance = std::abs((current_velocity * deacceleration_time) - (0.5 * max_acceleration * deacceleration_time * deacceleration_time));

  if(std::abs(error_position) > deacceleration_distance) current_velocity += get_sign(error_position) * max_acceleration * dt;
  else current_velocity -= get_sign(current_velocity) * max_acceleration * dt;

  if(current_velocity > target_velocity) current_velocity = target_velocity;
  else if(current_velocity < -target_velocity) current_velocity = -target_velocity;
  
  // if(std::abs(error_position) < target_pos_max_error) current_velocity = 0;

  previous_velocity = current_velocity;
  return current_velocity;
}


void BasicControler::begin_state(float current_position, float current_velocity, float current_time){
  previous_position = current_position;
  previous_time = current_time;
  current_velocity = current_velocity;
}

void BasicControler::set_max_acceleration(float max_acceleration){
  this->max_acceleration = max_acceleration;
};

void BasicControler::set_target_pos_max_error(float target_pos_max_error){
  this->target_pos_max_error = target_pos_max_error;
};