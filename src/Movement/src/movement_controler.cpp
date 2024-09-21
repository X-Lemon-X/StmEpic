
// #include "main.h"
#include "stm32f4xx_hal.h"
#include "movement_controler.hpp"
#include <cmath>

using namespace MOVEMENT_CONTROLER;


MovementEquation::MovementEquation(TIMING::Ticker &_ticker): ticker(_ticker){}

MovementControler::MovementControler(){
  initialized = false;
  target_position = 0;
  current_position = 0;
  target_velocity = 0;
  current_velocity = 0;
  dont_override_limit_position = true;
}

MovementControler::~MovementControler(){
  if(initialized){
    steper_motor->set_velocity(0.0);
    steper_motor->set_enable(false);
  }
}

void MovementControler::init(TIMING::Ticker &_ticker, STEPER_MOTOR::SteperMotor &_steper_motor, ENCODER::Encoder &_encoder_pos,ENCODER::Encoder &_encoder_velocity, MovementEquation &_movement_equation){
  ticker = &_ticker;
  steper_motor = &_steper_motor;
  encoder_pos = &_encoder_pos;
  encoder_vel = &_encoder_velocity;
  movement_equation = &_movement_equation;
  initialized = true;
  movement_equation->begin_state(encoder_pos->read_angle(), encoder_pos->get_velocity(), ticker->get_seconds());
}

void MovementControler::handle(){
  if (!initialized) return;
  current_position = encoder_pos->get_absoulute_angle();
  // current_velocity = encoder_vel->get_velocity();
  
  float new_velocity = movement_equation->calculate(current_position, target_position, current_velocity, target_velocity);
  // current_velocity = new_velocity;
  
  if (std::abs(new_velocity) > max_velocity)
    new_velocity = (new_velocity > 0) ? max_velocity : -max_velocity;

  if ( dont_override_limit_position && ( current_position < min_position || current_position > max_position)){
    new_velocity = 0.0;
    limit_positon_achieved = true;
  }else {
    limit_positon_achieved = false;
  }

  current_velocity = new_velocity;
  steper_motor->set_enable(enable);
  steper_motor->set_velocity(new_velocity);
}

void MovementControler::set_velocity(float velocity){
  target_velocity = velocity;
}

void MovementControler::set_enable(bool enable){
  this->enable = enable;
}

void MovementControler::set_position(float position){
  if(position < min_position)
    position = min_position;
  if(position > max_position)
    position = max_position;
  target_position = position;
}

void MovementControler::set_limit_position(float min_position, float max_position){
  this->min_position = min_position;
  this->max_position = max_position;
}

void MovementControler::set_max_velocity(float max_velocity){
  this->max_velocity = std::abs(max_velocity);
}


float MovementControler::get_current_position()const{
  return current_position;
}

float MovementControler::get_current_velocity()const{
  return current_velocity;
}

void MovementControler::override_limit_position(bool overide){
  dont_override_limit_position = !overide;
}

bool MovementControler::get_limit_position_achieved() const{
  return limit_positon_achieved;
}