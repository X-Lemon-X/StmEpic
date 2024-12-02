#include "movement_controler.hpp"


namespace stmepic{

class BasicLinearPosControler: public MovementEquation{
private:
  float max_acceleration;
  float target_pos_max_error;
  // float previous_velocity;
  // float previous_position;
  float previous_time;
  MovementState previous_state;
  MovementState current_state;

  float get_sign(float value);
public:
  BasicLinearPosControler(Ticker &ticker);
  
  void begin_state(MovementState current_state, float current_time) override;
  MovementState calculate(MovementState current_state, MovementState target_state) override;
  
  void set_max_acceleration(float max_acceleration);
  void set_target_pos_max_error(float target_pos_max_error);
};
  
} // namespace CONTROLER