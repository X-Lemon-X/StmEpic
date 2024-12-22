#include "movement_controler.hpp"


namespace stmepic{

class PIDControler: public MovementEquation{
private:
  float Kp;
  float Kd;
  float Ki;
  MovementState previous_state;
  float previous_time;
public:
  PIDControler();
  
   void begin_state(MovementState current_state, float current_time) override;
  MovementState calculate(MovementState  current_state, MovementState target_state) override;
  
  void set_Kp(float Kp){this->Kp = Kp;};
  void set_Kd(float Kd){this->Kd = Kd;};
  void set_Ki(float Ki){this->Ki = Ki;};
};
  
} // namespace CONTROLER
