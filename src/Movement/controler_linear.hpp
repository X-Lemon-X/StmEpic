#include "movement_controler.hpp"


/**
 * @file controler_linear.hpp
 * @brief Basic linear controler for position control.
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
 * @class BasicLinearPosControler
 * @brief Basic linear controler for position control.
 *
 * This class provides functionalities for controlling the movement of the actuator by position.
 *
 */
class BasicLinearPosControler : public MovementEquation {
private:
  float max_acceleration;
  float target_pos_max_error;
  // float previous_velocity;
  // float previous_position;
  float previous_time;
  MovementState previous_state;
  MovementState current_state;

  [[nodiscard]] float get_sign(float value);

public:
  BasicLinearPosControler();

  void begin_state(MovementState current_state, float current_time) override;
  [[nodiscard]] MovementState calculate(MovementState current_state, MovementState target_state) override;

  void set_max_acceleration(float max_acceleration);
  void set_target_pos_max_error(float target_pos_max_error);
};

} // namespace stmepic::movement