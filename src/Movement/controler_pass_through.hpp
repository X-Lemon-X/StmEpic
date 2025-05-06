#pragma once
#include "movement_controler.hpp"

/**
 * @file controler_pass_through.hpp
 * @brief Pass thruht control equasion incase of external controler algoritms or when motor actuator have it's own control algoritm.
 *
 */

/**
 * @defgroup Movement
 * @{
 */


/**
 * @defgroup Movement_Controller Controllers
 * @brief Control algorithms for movement controller like PID etc..
 *
 * @{
 */

/**
 * @defgroup PassThrough_Movement_Controller PassThrough
 * @brief PassThrough controller for remote control algorithms, for example through ROS.
 * @{
 */

namespace stmepic::movement {

/// @brief PassThroughControler is a controler that passes the target state without any modification.
class PassThroughControler : public MovementEquation {
public:
  PassThroughControler();
  void begin_state(MovementState current_state, float current_time) override;
  MovementState calculate(MovementState current_state, MovementState target_state) override;
};
} // namespace stmepic::movement
