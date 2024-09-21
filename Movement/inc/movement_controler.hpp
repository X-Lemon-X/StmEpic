#include "stm32f4xx_hal.h"
#include "Timing.hpp" 
#include "steper_motor.hpp"
#include "encoder.hpp"

#ifndef MOVE_CONTROLER_HPP
#define MOVE_CONTROLER_HPP

namespace MOVEMENT_CONTROLER{

class MovementEquation{
protected:  
  /// @brief  Ticker object for main clock time with microsecond resolution
  /// you can obatin the current time by calling ticker.get_seconds()
  TIMING::Ticker &ticker;
public:
  
  /// @brief Constructor for the MovementEquation class
  MovementEquation(TIMING::Ticker &ticker);

  /// @brief  Destructor for the MovementEquation class
  virtual ~MovementEquation(){};

  /// @brief initiates the begining state of the controler
  /// @param current_position current position of the arm in radians when the controler is initiated
  /// @param current_velocity current velocity of the arm in radians per second when the controler is initiated
  /// @param current_time current time in seconds when the controler is initiated
  virtual void begin_state(float current_position, float current_velocity, float current_time){};

  /// @brief This function should will be called in each pass of the MovementControler::handle() function
  /// @param current_position current position of the arm in radians
  /// @param target_position target position of the arm in radians
  /// @param current_velocity current velocity of the arm in radians per second
  /// @param target_velocity target angualar velocity of the arm in radians per second
  /// @return the angualar velocity of the arm in radians per second. 
  /// Can be positive or negative value (negative value obviously means reverse), 0 will stop the engine)
  virtual float calculate(float current_position, float target_position, float current_velocity, float target_velocity) {return 0.0f;};
};

class MovementControler{ 
private:
  TIMING::Ticker *ticker;
  STEPER_MOTOR::SteperMotor *steper_motor;
  ENCODER::Encoder *encoder_pos;
  ENCODER::Encoder *encoder_vel;
  MovementEquation *movement_equation;
  bool initialized;

  float max_velocity;
  float min_position;
  float max_position;


  float target_position;
  float current_position;
  float target_velocity;
  float current_velocity;
  bool enable;
  bool dont_override_limit_position;
  bool limit_positon_achieved;
public:

  /// @brief Arm controler interface
  MovementControler();

  ~MovementControler();
  
  /// @brief Initialize the controler/ shpuld be called after all the encoder, motor and ticker objects are initialized and ready to use
  /// @param ticker Ticker object for main system time wity microsecond resolution
  /// @param steper_motor Steper motor object
  /// @param encoder_pos encodere for a position of the arm (probablly mounted on the arm shaft)
  /// @param encoder_velocity encoder for the velocity of the arm (probablly mounted on the engine shaft)
  void init(TIMING::Ticker &ticker, STEPER_MOTOR::SteperMotor &steper_motor, ENCODER::Encoder &encoder_pos,ENCODER::Encoder &encoder_velocity, MovementEquation &movement_equation);
  
  /// @brief Handles all the caluclation and limits, this function should be called in the main loop as often as possible
  void handle();

  /// @brief Set the target velocity for the engine
  /// @param velocity Target velocity in rad/s
  void set_velocity(float velocity);

  /// @brief Enable or disable the engine
  /// @param enable True to turn on the endine-brake, false to turn off the engine-brake
  void set_enable(bool enable);

  /// @brief Set the target position for the engine
  /// @param position Target position in rad can be positive or negative
  void set_position(float position);

  /// @brief Set the limit position for arm
  /// @param min_position Minimum position in rad can be positive or negative
  /// @param max_position Maximum position in rad can be positive or negative have to be greater than min_position
  void set_limit_position(float min_position, float max_position);

  /// @brief Set the max velocity for the arm
  /// @param max_velocity Maximum velocity in rad/s
  void set_max_velocity(float max_velocity);

  /// @brief Get the current position of the arm
  /// @return Current position in rad
  float get_current_position() const;

  /// @brief Get the current velocity of the arm
  /// @return Current velocity in rad/s
  float get_current_velocity() const;

  bool get_limit_position_achieved() const;

  /// @brief ovveride the limit position by turning off the limit position
  /// definitely not recommended to make it true for a regular use since it can damage the arm
  /// @param override True to turn off the limit position, false to turn on the limit position
  void override_limit_position(bool override);
};





} // namespace ENGINE_CONTROLER
#endif