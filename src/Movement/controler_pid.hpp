#include "movement_controler.hpp"
#include "pid.hpp"

/**
 * @file controler_pid.hpp
 * @brief class definition which provides functionalities for controlling the movement of actuators.
 *
 */

/**
 * @defgroup Movement
 * @{
 */


/**
 * @defgroup Movement_Controller Controllers
 * @{
 */


/**
 * @defgroup PID_Controller PID
 * @brief PID controller.
 * @{
 */

namespace stmepic::movement {

/**
 * @class PIDController
 * @brief MiniPID-like controller adapted to MovementState (position control).
 *
 * This implements the MiniPID API adapted for position control where the "actual"
 * value is `current_state.position` and the setpoint is `target_state.position`.
 */
class PIDController : public MovementEquation {


public:
  PIDController();

  void begin_state(MovementState current_state, float current_time) override;
  MovementState calculate(MovementState current_state, MovementState target_state) override;

  void set_velocity_pid_config(const stmepic::controller::PidConfig &cfg);
  void set_position_pid_config(const stmepic::controller::PidConfig &cfg);
  void set_torque_pid_config(const stmepic::controller::PidConfig &cfg);

private:
  MovementState previous_state;
  float previous_time;
  stmepic::controller::Pid velocity_pid;
  stmepic::controller::Pid position_pid;
  stmepic::controller::Pid torque_pid;
};

} // namespace stmepic::movement
