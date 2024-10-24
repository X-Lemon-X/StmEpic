#include "controler_pass_through.hpp"


namespace stmepic{

  PassThroughControler::PassThroughControler(Ticker &ticker): MovementEquation(ticker){
  }
  
  void PassThroughControler::begin_state(float current_position, float current_velocity, float current_time){
    // do nothing
  }
  
  float PassThroughControler::calculate(float current_position, float target_position, float current_velocity, float target_velocity){
    return target_velocity;
  }
} // namespace CONTROLER