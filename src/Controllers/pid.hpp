#pragma once
#include "stmepic.hpp"

// based on MiniPID https://github.com/tekdemo/MiniPID


/**
 * @file pid.hpp
 * @brief This file contains the pid class definition.
 */


/**
 * @defgroup Controllers
 * @brief Base classes for controllers like PID and so on.
 * @{
 */

namespace stmepic::controller {

struct PidConfig {
  double p              = 1.0;
  double i              = 0.0;
  double d              = 0.0;
  double f              = 0.0;
  double maxIOutput     = 0.0;
  double maxOutput      = 0.0;
  double minOutput      = 0.0;
  bool reversed         = false;
  double outputRampRate = 0.0;
  double outputFilter   = 0.0;
  double setpointRange  = 0.0;
};

class Pid {
public:
  Pid();
  Pid(double p, double i, double d);
  Pid(double p, double i, double d, double f);
  Pid(const PidConfig &config);

  /**
   * Configure the Proportional gain parameter. <br>
   * this->responds quicly to changes in setpoint, and provides most of the initial driving force
   * to make corrections. <br>
   * Some systems can be used with only a P gain, and many can be operated with only PI.<br>
   * For position based controllers, this->is the first parameter to tune, with I second. <br>
   * For rate controlled systems, this->is often the second after F.
   *
   * @param p Proportional gain. Affects output according to <b>output+=P*(setpoint-current_value)</b>
   */
  void setP(double p);

  /**
   * Changes the I parameter <br>
   * this->is used for overcoming disturbances, and ensuring that the controller always gets to the control
   * mode. Typically tuned second for "Position" based modes, and third for "Rate" or continuous based modes.
   * <br> Affects output through <b>output+=previous_errors*Igain ;previous_errors+=current_error</b>
   *
   * @see {@link #setMaxIOutput(double) setMaxIOutput} for how to restrict
   *
   * @param i New gain value for the Integral term
   */
  void setI(double i);
  void setD(double d);

  /**Configure the FeedForward parameter. <br>
   * this->is excellent for Velocity, rate, and other	continuous control modes where you can
   * expect a rough output value based solely on the setpoint.<br>
   * Should not be used in "position" based control modes.
   *
   * @param f Feed forward gain. Affects output according to <b>output+=F*Setpoint</b>;
   */
  void setF(double f);

  /** Create a new PID object.
   * @param p Proportional gain. Large if large difference between setpoint and target.
   * @param i Integral gain.	Becomes large if setpoint cannot reach target quickly.
   * @param d Derivative gain. Responds quickly to large changes in error. Small values prevents P and I terms from causing overshoot.
   */
  void setPID(double p, double i, double d);
  void setPID(double p, double i, double d, double f);

  /**Set the maximum output value contributed by the I component of the system
   * this->can be used to prevent large windup issues and make tuning simpler
   * @param maximum. Units are the same as the expected output value
   */
  void setMaxIOutput(double max);

  /**Specify a maximum output. If a single parameter is specified, the minimum is
   * set to (-maximum).
   * @param output
   */
  void setOutputLimits(double limit);

  /**
   * Specify a maximum output.
   * @param minimum possible output value
   * @param maximum possible output value
   */
  void setOutputLimits(double min, double max);

  /** Set the operating direction of the PID controller
   * @param reversed Set true to reverse PID output
   */
  void setDirection(bool reversed);

  // configure/get full config
  void setConfig(const PidConfig &cfg);

  const PidConfig &getConfig() const;

  /**
   * @brief
   * Set the target for the PID calculations
   * @param setpoint
   */
  void setSetpoint(double);

  void reset();

  void setOutputRampRate(double);

  /**
   * @brief Set a limit on how far the setpoint can be from the current position
   * <br>Can simplify tuning by helping tuning over a small range applies to a much larger range.
   * <br>this->limits the reactivity of P term, and restricts impact of large D term
   * during large setpoint adjustments. Increases lag and I term if range is too small.
   * @param range
   */
  void setSetpointRange(double);

  void setOutputFilter(double);

  double getOutput();

  double getOutput(double);

  double getOutput(double, double);

private:
  double clamp(double, double, double);
  bool bounded(double, double, double);
  void checkSigns();
  void init();
  PidConfig conf;

  // runtime states
  double errorSum;
  double lastActual;
  double lastOutput;
  double setpoint;
  bool firstRun;
};

} // namespace stmepic::controller