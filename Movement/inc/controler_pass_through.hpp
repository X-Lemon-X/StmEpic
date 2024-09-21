
#ifndef __PASS_THROUGH_CONTROLER_HPP
#define __PASS_THROUGH_CONTROLER_HPP

#include "movement_controler.hpp"

namespace CONTROLER{
  /// @brief PassThroughControler is a controler that passes the target velocity to the motor without any modification
  class PassThroughControler: public MOVEMENT_CONTROLER::MovementEquation{
  public:
    PassThroughControler(TIMING::Ticker &ticker);
    void begin_state(float current_position, float current_velocity, float current_time) override;
    float calculate(float current_position, float target_position, float current_velocity, float target_velocity) override;
  };
} // namespace CONTROLER

#endif // __PASS_THROUGH_CONTROLER_HPP