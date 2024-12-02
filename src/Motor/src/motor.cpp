

#include "motor.hpp"
#include "stmepic.hpp"

using namespace stmepic;

MotorBaseClosedLoop::MotorBaseClosedLoop(MotorBase &_motor, Encoder *_encoder_pos, Encoder *_encoder_vel):
motor(_motor), encoder_pos(_encoder_pos), encoder_vel(_encoder_vel){}

void MotorBaseClosedLoop::init(){
  motor.init();
}

float MotorBaseClosedLoop::get_velocity() const{
  if(encoder_vel != nullptr) return encoder_vel->get_velocity();
  else return motor.get_velocity();
}

float MotorBaseClosedLoop::get_torque() const{
  return motor.get_torque();
}

float MotorBaseClosedLoop::get_position() const{
  if(encoder_pos != nullptr) return encoder_pos->get_angle();
  else return motor.get_position();
}

float MotorBaseClosedLoop::get_absolute_position() const{
  if(encoder_pos != nullptr) return encoder_pos->get_absoulute_angle();
  else return motor.get_absolute_position();
}

float MotorBaseClosedLoop::get_gear_ratio() const{
  return motor.get_gear_ratio();
}

void MotorBaseClosedLoop::set_velocity(float speed){
  motor.set_velocity(speed);
}

void MotorBaseClosedLoop::set_torque(float torque){
  motor.set_torque(torque);
}

void MotorBaseClosedLoop::set_position(float position){
  motor.set_position(position);
}

void MotorBaseClosedLoop::set_enable(bool enable){
  motor.set_enable(enable);
}

void MotorBaseClosedLoop::set_gear_ratio(float gear_ratio){
  motor.set_gear_ratio(gear_ratio);
}

void MotorBaseClosedLoop::set_max_velocity(float max_velocity){
  motor.set_max_velocity(max_velocity);
}

void MotorBaseClosedLoop::set_min_velocity(float min_velocity){
  motor.set_min_velocity(min_velocity);
}

void MotorBaseClosedLoop::set_reverse(bool reverse){
  motor.set_reverse(reverse);
}
