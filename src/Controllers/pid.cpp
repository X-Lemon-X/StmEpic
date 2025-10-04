

#include "pid.hpp"


using namespace stmepic;
using namespace stmepic::controller;


Pid::Pid(double p, double i, double d) {
  init();
  conf.p = p;
  conf.i = i;
  conf.d = d;
}
Pid::Pid(double p, double i, double d, double f) {
  init();
  conf.p = p;
  conf.i = i;
  conf.d = d;
  conf.f = f;
}
Pid::Pid(const PidConfig &config) {
  init();
  conf = config;
}
void Pid::init() {
  // default configuration
  conf = PidConfig{};
  // runtime state
  errorSum   = 0;
  lastActual = 0;
  lastOutput = 0;
  setpoint   = 0;
  firstRun   = true;
}

//**********************************
// Configuration functions
//**********************************

void Pid::setP(double p) {
  conf.p = p;
  checkSigns();
}


void Pid::setI(double i) {
  if(conf.i != 0) {
    errorSum = errorSum * conf.i / i;
  }
  if(conf.maxIOutput != 0) {
    // nothing to store, will compute maxError on demand
  }
  conf.i = i;
  checkSigns();
  /* Implementation note:
   * this->Scales the accumulated error to avoid output errors.
   * As an example doubling the I term cuts the accumulated error in half, which results in the
   * output change due to the I term constant during the transition.
   *
   */
}

void Pid::setD(double d) {
  conf.d = d;
  checkSigns();
}


void Pid::setF(double f) {
  conf.f = f;
  checkSigns();
}


void Pid::setPID(double p, double i, double d) {
  setP(p);
  setI(i);
  setD(d);
}

void Pid::setPID(double p, double i, double d, double f) {
  setP(p);
  setI(i);
  setD(d);
  setF(f);
}


void Pid::setMaxIOutput(double maximum) {
  /* Internally maxError and Izone are similar, but scaled for different purposes.
   * The maxError is generated for simplifying math, since calculations against
   * the max error are far more common than changing the I term or Izone.
   */
  conf.maxIOutput = maximum;
}


void Pid::setOutputLimits(double output) {
  setOutputLimits(-output, output);
}


void Pid::setOutputLimits(double minimum, double maximum) {
  if(maximum < minimum)
    return;
  conf.maxOutput = maximum;
  conf.minOutput = minimum;
  if(conf.maxIOutput == 0 || conf.maxIOutput > (maximum - minimum)) {
    setMaxIOutput(maximum - minimum);
  }
}


void Pid::setDirection(bool reversed) {
  conf.reversed = reversed;
}

void Pid::setConfig(const PidConfig &cfg) {
  setPID(cfg.p, cfg.i, cfg.d);
  setF(cfg.f);
  setMaxIOutput(cfg.maxIOutput);
  setOutputLimits(cfg.minOutput, cfg.maxOutput);
  setDirection(cfg.reversed);
  setOutputRampRate(cfg.outputRampRate);
  setOutputFilter(cfg.outputFilter);
  setSetpointRange(cfg.setpointRange);
}

const PidConfig &Pid::getConfig() const {
  return conf;
}

//**********************************
// Primary operating functions
//**********************************

/**Set the target for the PID calculations
 * @param setpoint
 */
void Pid::setSetpoint(double setpoint) {
  this->setpoint = setpoint;
}

/** Calculate the PID value needed to hit the target setpoint.
 * Automatically re-calculates the output at each call.
 * @param actual The monitored value
 * @param target The target value
 * @return calculated output value for driving the actual to the target
 */
double Pid::getOutput(double actual, double setpoint) {
  double output;
  double Poutput;
  double Ioutput;
  double Doutput;
  double Foutput;

  this->setpoint = setpoint;

  // Ramp the setpoint used for calculations if user has opted to do so
  if(conf.setpointRange != 0) {
    setpoint = clamp(setpoint, actual - conf.setpointRange, actual + conf.setpointRange);
  }

  // Do the simple parts of the calculations
  double error = setpoint - actual;

  // Calculate F output. Notice, this->depends only on the setpoint, and not the error.
  Foutput = conf.f * setpoint;

  // Calculate P term
  Poutput = conf.p * error;

  // If this->is our first time running this-> we don't actually _have_ a previous input or output.
  // For sensor, sanely assume it was exactly where it is now.
  // For last output, we can assume it's the current time-independent outputs.
  if(firstRun) {
    lastActual = actual;
    lastOutput = Poutput + Foutput;
    firstRun   = false;
  }


  // Calculate D Term
  // Note, this->is negative. this->actually "slows" the system if it's doing
  // the correct thing, and small values helps prevent output spikes and overshoot

  Doutput    = -conf.d * (actual - lastActual);
  lastActual = actual;


  // The Iterm is more complex. There's several things to factor in to make it easier to deal with.
  //  1. maxIoutput restricts the amount of output contributed by the Iterm.
  //  2. prevent windup by not increasing errorSum if we're already running against our max Ioutput
  //  3. prevent windup by not increasing errorSum if output is output=maxOutput
  Ioutput = conf.i * errorSum;
  if(conf.maxIOutput != 0) {
    Ioutput = clamp(Ioutput, -conf.maxIOutput, conf.maxIOutput);
  }

  // And, finally, we can just add the terms up
  output = Foutput + Poutput + Ioutput + Doutput;

  // Figure out what we're doing with the error.
  if(conf.minOutput != conf.maxOutput && !bounded(output, conf.minOutput, conf.maxOutput)) {
    errorSum = error;
    // reset the error sum to a sane level
    // Setting to current error ensures a smooth transition when the P term
    // decreases enough for the I term to start acting upon the controller
    // From that point the I term will build up as would be expected
  } else if(conf.outputRampRate != 0 && !bounded(output, lastOutput - conf.outputRampRate, lastOutput + conf.outputRampRate)) {
    errorSum = error;
  } else if(conf.maxIOutput != 0) {
    double maxError = (conf.i != 0) ? (conf.maxIOutput / conf.i) : 0.0;
    errorSum        = clamp(errorSum + error, -maxError, maxError);
    // In addition to output limiting directly, we also want to prevent I term
    // buildup, so restrict the error directly
  } else {
    errorSum += error;
  }

  // Restrict output to our specified output and ramp limits
  if(conf.outputRampRate != 0) {
    output = clamp(output, lastOutput - conf.outputRampRate, lastOutput + conf.outputRampRate);
  }
  if(conf.minOutput != conf.maxOutput) {
    output = clamp(output, conf.minOutput, conf.maxOutput);
  }
  if(conf.outputFilter != 0) {
    output = lastOutput * conf.outputFilter + output * (1 - conf.outputFilter);
  }

  lastOutput = output;
  return output;
}

/**
 * Calculates the PID value using the last provided setpoint and actual valuess
 * @return calculated output value for driving the actual to the target
 */
double Pid::getOutput() {
  return getOutput(lastActual, setpoint);
}

/**
 *
 * @param actual
 * @return calculated output value for driving the actual to the target
 */
double Pid::getOutput(double actual) {
  return getOutput(actual, setpoint);
}

/**
 * Resets the controller. this->erases the I term buildup, and removes D gain on the next loop.
 */
void Pid::reset() {
  firstRun = true;
  errorSum = 0;
}

/**Set the maximum rate the output can increase per cycle.
 * @param rate
 */
void Pid::setOutputRampRate(double rate) {
  conf.outputRampRate = rate;
}


void Pid::setSetpointRange(double range) {
  conf.setpointRange = range;
}

/**Set a filter on the output to reduce sharp oscillations. <br>
 * 0.1 is likely a sane starting value. Larger values P and D oscillations, but force larger I values.
 * Uses an exponential rolling sum filter, according to a simple <br>
 * <pre>output*(1-strength)*sum(0..n){output*strength^n}</pre>
 * @param output valid between [0..1), meaning [current output only.. historical output only)
 */
void Pid::setOutputFilter(double strength) {
  if(strength == 0 || bounded(strength, 0, 1)) {
    conf.outputFilter = strength;
  }
}

//**************************************
// Helper functions
//**************************************

/**
 * Forces a value into a specific range
 * @param value input value
 * @param min maximum returned value
 * @param max minimum value in range
 * @return Value if it's within provided range, min or max otherwise
 */
double Pid::clamp(double value, double min, double max) {
  if(value > max) {
    return max;
  }
  if(value < min) {
    return min;
  }
  return value;
}

/**
 * Test if the value is within the min and max, inclusive
 * @param value to test
 * @param min Minimum value of range
 * @param max Maximum value of range
 * @return
 */
bool Pid::bounded(double value, double min, double max) {
  return (min < value) && (value < max);
}

/**
 * To operate correctly, all PID parameters require the same sign,
 * with that sign depending on the {@literal}reversed value
 */
void Pid::checkSigns() {
  if(conf.reversed) { // all values should be below zero
    if(conf.p > 0)
      conf.p *= -1;
    if(conf.i > 0)
      conf.i *= -1;
    if(conf.d > 0)
      conf.d *= -1;
    if(conf.f > 0)
      conf.f *= -1;
  } else { // all values should be above zero
    if(conf.p < 0)
      conf.p *= -1;
    if(conf.i < 0)
      conf.i *= -1;
    if(conf.d < 0)
      conf.d *= -1;
    if(conf.f < 0)
      conf.f *= -1;
  }
}