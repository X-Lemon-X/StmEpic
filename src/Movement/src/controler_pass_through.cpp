#include "controler_pass_through.hpp"

using namespace stmepic;
using namespace stmepic::movement;
using namespace stmepic::gpio;

PassThroughControler::PassThroughControler () : MovementEquation () {
}

void PassThroughControler::begin_state (MovementState current_state, float current_time) {
  // do nothing
}

MovementState PassThroughControler::calculate (MovementState current_state, MovementState target_state) {
  return target_state;
}