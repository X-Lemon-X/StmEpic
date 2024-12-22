
#ifndef __PASS_THROUGH_CONTROLER_HPP
#define __PASS_THROUGH_CONTROLER_HPP

#include "movement_controler.hpp"

namespace stmepic{
  /// @brief PassThroughControler is a controler that passes the target velocity to the motor without any modification
  class PassThroughControler: public MovementEquation{
  public:
    PassThroughControler();
    void begin_state(MovementState current_state, float current_time) override;
    MovementState calculate(MovementState current_state, MovementState target_state) override;
  };
} // namespace CONTROLER

#endif // __PASS_THROUGH_CONTROLER_HPP