#include "stmepic.hpp"
#include "servo_motor.hpp"

using namespace stmepic::motor;
using namespace stmepic;


ServoMotorPWM::ServoMotorPWM(TIM_HandleTypeDef &_htim, unsigned int _timer_channel)
: htim(_htim), timer_channel(_timer_channel), gear_ratio(1.0f), reverse(false), enable(false),
  status(Status::ExecutionError("ServoMotorPWM not initialized")) {

  ServoMotorPWMSettings default_settings;
  default_settings.min_pulse_width_us = 500.0f;
  default_settings.max_pulse_width_us = 2500.0f;
  default_settings.pwm_frequency      = 330.0f;
  default_settings.min_angle_rad      = 0.0f;
  default_settings.max_angle_rad      = 3.14f; // 180 degrees in radians
  default_settings.n_multiplayer      = 1;     // Default multiplier for resolution
  (void)device_set_settings(default_settings);
}

float ServoMotorPWM::timer_values_to_freq(uint32_t prescaler, uint32_t counter_max, uint32_t timer_clock) {
  return static_cast<float>(timer_clock) / ((prescaler + 1) * (counter_max + 1));
}

struct TimerConfig {
  uint32_t prescaler;
  uint32_t counter;
  double achieved_freq;
};

TimerConfig find_best_timer_config(double base_clk, double target_freq, uint32_t max_prescaler, uint32_t max_counter) {
  TimerConfig best = { 1, 1, base_clk };
  double min_error = std::abs(base_clk - target_freq);

  for(uint32_t prescaler = 1; prescaler <= max_prescaler; ++prescaler) {

    uint32_t counter = static_cast<uint32_t>(base_clk / ((prescaler + 1) * target_freq));
    if(counter < 1 || counter > max_counter)
      continue;

    double freq  = base_clk / (prescaler * counter);
    double error = std::abs(freq - target_freq);

    if(error < min_error && (counter > best.counter || std::abs(error - min_error) < 1e-3)) {
      min_error          = error;
      best.prescaler     = prescaler;
      best.counter       = counter;
      best.achieved_freq = freq;
    }
  }
  return best;
}

Status ServoMotorPWM::device_start() {
  // uint32_t counter_max  = 65535 / settings.n_multiplayer; // Max counter value for 16-bit timer
  // float prescaler_float = HAL_RCC_GetSysClockFreq() / (settings.pwm_frequency * counter_max);
  // uint32_t prescaler    = (uint32_t)prescaler_float;
  TimerConfig config = find_best_timer_config(HAL_RCC_GetSysClockFreq(), settings.pwm_frequency, 65535, 65535);


  if(config.prescaler > 65535) {
    return Status::Invalid("ServoMotorPWM: Invalid prescaler value (must be between 0 and 65535)");
  }

  if(config.prescaler == 0) {
    return Status::Invalid("ServoMotorPWM: Prescaler cannot be zero | this means that the PWM frequency is "
                           "too high for the timer clock, decrease the n_multiplayer");
  }

  float cpr = ((settings.max_pulse_width_us - settings.min_pulse_width_us) / 1000000.0f) /
              (settings.max_angle_rad - settings.min_angle_rad);
  if(cpr <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid cpr (must be > 0)");
  }
  count_per_rad = cpr * settings.pwm_frequency * config.counter;
  min_pulse_width =
  static_cast<uint32_t>((float)(settings.min_pulse_width_us / 1000000.0f * settings.pwm_frequency * config.counter));

  __HAL_TIM_SET_PRESCALER(&htim, config.prescaler); // Set the prescaler in the timer handle
  __HAL_TIM_SET_AUTORELOAD(&htim, config.counter);  // Set the auto-reload value in the timer handle
  set_enable(false);

  // Start the PWM signal generation
  if(HAL_TIM_PWM_Start(&htim, static_cast<uint32_t>(timer_channel)) != HAL_OK) {
    status = Status::HalError("Failed to start PWM");
  } else {
    status = Status::OK();
  }
  return status;
}

void ServoMotorPWM::set_velocity(float speed) {
  (void)speed; // Servo motors are not speed controlled, so this function does nothing
}

void ServoMotorPWM::set_torque(float torque) {
  (void)torque; // Servo motors are not torque controlled, so this function does nothing
}

void ServoMotorPWM::set_enable(bool enable) {
  this->enable = enable;
  __HAL_TIM_SET_COMPARE(&htim, static_cast<uint32_t>(timer_channel), 0);
}

void ServoMotorPWM::set_gear_ratio(float gear_ratio) {
  this->gear_ratio = gear_ratio;
}

void ServoMotorPWM::set_reverse(bool reverse) {
  this->reverse = reverse;
}

void ServoMotorPWM::set_max_velocity(float max_velocity) {
  (void)max_velocity; // Servo motors do not have a velocity setting, so this function does nothing
}


void ServoMotorPWM::set_min_velocity(float min_velocity) {
  (void)min_velocity; // Servo motors do not have a velocity setting, so this function does nothing
}


void ServoMotorPWM::set_position(float position) {
  if(!enable)
    return;

  position *= gear_ratio;

  if(reverse) {
    position = settings.max_angle_rad - position; // Reverse the position if reverse is enabled
  }

  if(position < settings.min_angle_rad) {
    status   = Status::Invalid("ServoMotorPWM: Position out of bounds, must be >= min_angle_rad");
    position = settings.min_angle_rad;
  } else if(position > settings.max_angle_rad) {
    status   = Status::Invalid("ServoMotorPWM: Position out of bounds, must be <= max_angle_rad");
    position = settings.max_angle_rad;
  } else {
    status = Status::OK();
  }

  uint32_t pulse_count = min_pulse_width + static_cast<uint32_t>((float)(position * count_per_rad));
  __HAL_TIM_SET_COMPARE(&htim, static_cast<uint32_t>(timer_channel), pulse_count);
  postion = position; // Update the internal position state
}

float ServoMotorPWM::get_velocity() const {
  return 0.0f; // Servo motors are not speed controlled, so always return 0
}

float ServoMotorPWM::get_torque() const {
  return 0.0f; // Servo motors are not torque controlled, so always return 0
}

float ServoMotorPWM::get_position() const {
  return postion; // Return the current position in radians
}

float ServoMotorPWM::get_absolute_position() const {
  return postion; // Absolute position is the same as the current position for servo motors
}

float ServoMotorPWM::get_gear_ratio() const {
  return gear_ratio; // Return the current gear ratio
}

Status ServoMotorPWM::device_stop() {
  // Stop the PWM signal generation
  // This is usually done by stopping the timer and disabling the PWM output
  if(HAL_TIM_PWM_Stop(&htim, static_cast<uint32_t>(timer_channel)) != HAL_OK) {
    status = Status::HalError("Failed to stop PWM");
  } else {
    status = Status::OK();
  }
  return status;
}

Status ServoMotorPWM::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  return device_start();
}

bool ServoMotorPWM::device_ok() {
  return status.ok();
}

Result<bool> ServoMotorPWM::device_is_connected() {
  return Result<bool>::OK(true);
}

Status ServoMotorPWM::device_get_status() {
  return status;
}

Status ServoMotorPWM::device_set_settings(const DeviceSettings &_settings) {
  const auto *servo_settings = dynamic_cast<const ServoMotorPWMSettings *>(&_settings);
  if(!servo_settings) {
    return Status::TypeError("ServoMotorPWM: Invalid settings type, expected ServoMotorPWMSettings");
  }
  if(servo_settings->pwm_frequency <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid pwm_frequency (must be > 0)");
  }
  if(servo_settings->min_pulse_width_us <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid min_pulse_width_us (must be > 0)");
  }
  if(servo_settings->max_pulse_width_us <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid max_pulse_width_us (must be > 0)");
  }
  if(servo_settings->min_angle_rad < 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid min_angle_rad (must be >= 0)");
  }
  if(servo_settings->max_angle_rad <= servo_settings->min_angle_rad) {
    return Status::Invalid("ServoMotorPWM: max_angle_rad must be greater than min_angle_rad");
  }
  if(servo_settings->n_multiplayer < 1) {
    return Status::Invalid("ServoMotorPWM: n_multiplayer must be >= 1");
  }
  settings = *servo_settings;
  return Status::OK();
}