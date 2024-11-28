#include "controler_pass_through.hpp"


namespace stmepic{

  PassThroughControler::PassThroughControler(Ticker &ticker): MovementEquation(ticker){
  }
  
  void PassThroughControler::begin_state(MovementState current_state, float current_time){
    // do nothing
  }
  
  MovementState PassThroughControler::calculate(MovementState current_state, MovementState target_state){
    return target_state;
  }
} // namespace CONTROLER