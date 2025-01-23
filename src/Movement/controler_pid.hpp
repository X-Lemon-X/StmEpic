#include "movement_controler.hpp"


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
 * @defgroup Movement_Controler Controlers
 * @{
 */

namespace stmepic::movement {

/**
 * @class PIDControler
 * @brief Class representing the PID controler.
 *
 * PID controler.
 *
 */
class PIDControler : public MovementEquation {
private:
  float Kp;
  float Kd;
  float Ki;
  MovementState previous_state;
  float previous_time;

public:
  PIDControler();

  void begin_state(MovementState current_state, float current_time) override;
  MovementState calculate(MovementState current_state, MovementState target_state) override;

  /**
   * @brief Set the Kp value for the PID controler
   * @param Kp Kp value
   */
  void set_Kp(float Kp) {
    this->Kp = Kp;
  };

  /**
   * @brief Set the Kd value for the PID controler
   * @param Kd Kd value
   */
  void set_Kd(float Kd) {
    this->Kd = Kd;
  };

  /**
   * @brief Set the Ki value for the PID controler
   * @param Ki Ki value
   */
  void set_Ki(float Ki) {
    this->Ki = Ki;
  };
};

} // namespace stmepic::movement
