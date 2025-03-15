#pragma once
#include "Timing.hpp"
#include "encoder.hpp"
#include "motor.hpp"
#include "stmepic.hpp"
#include "simple_task.hpp"

/**
 * @file movement_controler.hpp
 * @brief class definition which provides functionalities for controlling the movement of actuators.
 *
 */

/**
 * @defgroup Movement
 * @brief Functions wrapers for actuators, control algorithms, encodrs, and movement control handlers for closed/open loop systems.
 * @{
 */

namespace stmepic::movement {

/**
 * @enum MovementControlMode
 * @brief Enumeration representing various modes of movement control.
 *
 * @var MovementControlMode::POSITION
 * Control the movement by position.
 *
 * @var MovementControlMode::VELOCITY
 * Control the movement by velocity.
 *
 * @var MovementControlMode::TORQUE
 * Control the movement by torque.
 */
enum class MovementControlMode { POSITION, VELOCITY, TORQUE };

/**
 * @class MovementState
 * @brief Class representing the state of the actuator.
 *
 * This class provides functionalities for representing the state of the actuator.
 *
 */
struct MovementState {
  float position;
  float velocity;
  float torque;
};

/**
 * @class MovementEquation
 * @brief Class representing the movement equation.
 *
 * This class provides interfaces for the equasions used to control the behavior of the actuator.
 *
 */
class MovementEquation {
public:
  /// @brief Constructor for the MovementEquation class
  MovementEquation();

  /// @brief  Destructor for the MovementEquation class
  virtual ~MovementEquation(){};

  /// @brief initiates the begining state of the controler
  /// @param current_position current position of the arm in radians when the controler is initiated
  /// @param current_velocity current velocity of the arm in radians per second when the controler is initiated
  /// @param current_time current time in seconds when the controler is initiated
  virtual void begin_state(MovementState current_state, float current_time) {
    (void)current_state;
    (void)current_time;
  };

  /// @brief This function should will be called in each pass of the MovementControler::handle() function
  /// @param current_position current position of the arm in radians
  /// @param target_position target position of the arm in radians
  /// @param current_velocity current velocity of the arm in radians per second
  /// @param target_velocity target angualar velocity of the arm in radians per second
  /// @return the angualar velocity of the arm in radians per second.
  /// Can be positive or negative value (negative value obviously means reverse), 0 will stop the engine)
  [[nodiscard]] virtual MovementState calculate(MovementState current_state, MovementState target_state) {
    (void)current_state;
    (void)target_state;
    return MovementState{ 0, 0, 0 };
  };
};

/**
 * @class MovementControler
 * @brief Class representing the movement controler.
 *
 * This class provides functionalities for controlling the movement of actuators.
 *
 */
class MovementControler {
public:
  /// @brief Arm controler interface
  MovementControler();

  ~MovementControler();

  /// @brief Initialize the controler/ shpuld be called after all the encoder, motor and
  /// ticker objects are initialized and ready to use
  /// this will also start a  task that will handle the controler
  /// @param ticker Ticker object for main system time wity microsecond resolution
  /// @param motor engine that will be controlled by the controler
  /// @param control_mode MovementControlMode mode of the controler this will determine
  /// how the data from the movement equation will inputed to the engine for example if
  /// the control_mode is set to VELOCITY the set_velocity function will be used call on
  /// motor object, this can be mixed with the MovementState returned by MovementEquation
  /// to use velocity to control engine that can be controled only by position or torque.
  /// @param movement_equation MovementEquation equasion that will be used to calculate
  /// the controls for the engine
  /// @param encoder_velocity encoder for the velocity of the arm (probablly mounted on
  /// the engine shaft), if passed as nullptr the current velocity will be owerriden by
  /// the movement equation. encoder_velocity was added for super precise velocity control
  /// when you have two encoders one on the engine and one on the other shaft. However if
  /// second encoder is not used it is recommended to pass
  // the pass the same encoder_velocity as the encoder_pos.
  void init(std::shared_ptr<motor::MotorBase> motor,
            MovementControlMode control_mode,
            std::shared_ptr<MovementEquation> movement_equation);

  /// @brief Set the target velocity for the engine
  /// @param velocity Target velocity in rad/s
  void set_velocity(float velocity);

  /// @brief Get the current position of the Motor
  /// @return The position in radians

  /// @brief Set the target torque for the engine
  /// @param torque Target torque in Nm
  void set_torque(float torque);

  /// @brief Enable or disable the engine
  /// @param enable True to turn ON the motor false to turn OFF the motor
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

  /// @brief Set the max torque for the arm
  /// @param max_torque Maximum torque in Nm
  void set_max_torque(float max_torque);

  /// @brief Get the current position of the arm
  /// @return Current position in rad
  [[nodiscard]] float get_current_position() const;

  /// @brief Get the current velocity of the arm
  /// @return Current velocity in rad/s
  [[nodiscard]] float get_current_velocity() const;

  /// @brief Get the current torque of the arm
  /// @return Current torque in Nm
  [[nodiscard]] float get_current_torque() const;

  [[nodiscard]] bool get_limit_position_achieved() const;

  /// @brief Get the current torque of the Motor
  /// @return The torque in Nm
  [[nodiscard]] float get_torque() const;

  /// @brief Get if the motor is enabled
  /// In most cases motor might get disbaled if error occurs on motor or encoders
  [[nodiscard]] bool get_enable() const;

  /// @brief overide the limit position by turning off the limit position
  /// definitely not recommended to make it true for a regular use since it can damage the arm
  /// @param override True to turn off the limit position, false to turn on the limit position
  void override_limit_position(bool override);

private:
  // motor::MotorBase *motor;
  std::shared_ptr<motor::MotorBase> motor;
  std::shared_ptr<MovementEquation> movement_equation;
  MovementControlMode control_mode;
  bool initialized;
  SimpleTask task;

  float max_velocity;
  float min_position;
  float max_position;
  float max_torque;
  MovementState current_state;
  MovementState target_state;

  bool enable;
  bool dont_override_limit_position;
  bool limit_positon_achieved;

  [[nodiscard]] static float overide_limit_abs(float value, float max, float min = 0);
  [[nodiscard]] static float overide_limit(float value, float max, float min = 0);

  void set_motor_state(MovementState state);

  void handle_internal();

  /// @brief Handles all the caluclation and limits, this function should be called in the main loop as often as possible
  static void handle(SimpleTask &task, void *args);
};


} // namespace stmepic::movement
