
#include "pid_controler.hpp"
#include <cmath>

using namespace CONTROLER;


PIDControler::PIDControler(TIMING::Ticker &ticker): MOVEMENT_CONTROLER::MovementEquation(ticker){
  Kp = 0;
  Kd = 0;
  Ki = 0;
  previous_velocity = 0;
  previous_position = 0;
  previous_time = 0;
}

float PIDControler::calculate(float current_position, float target_position, float current_velocity, float target_velocity){  
  const float current_time = ticker.get_seconds();
  // const float state_velocity = (previous_position-current_position) / (current_time - previous_time);
  float dt = current_time - previous_time;
  previous_time = current_time;
  // return Kp * (target_position - current_position) + Kd * (target_velocity - state_velocity);
  // float error = target_velocity - current_velocity;
  // add inertia to the system
  // float error_position= target_position - current_position;
  // float error_velocity = target_velocity - current_velocity;
  // float velocity =  (Kp * error_position) + (Kd * error_position/dt);
  // float error_velocity = target_velocity - current_velocity;
  // float error_i = ;
  // float velocity =  (Kp * previous_velocity) + (Kd * target_velocity);
  float next_position = (Kd * current_position) + (target_position * Ki);
  float velocity = (next_position - previous_position) / dt;
  previous_position = next_position;
  previous_velocity = current_velocity;
  return velocity;
}


void PIDControler::begin_state(float current_position, float current_velocity, float current_time){
  previous_position = current_position;
  previous_time = current_time;
  current_velocity = current_velocity;
}