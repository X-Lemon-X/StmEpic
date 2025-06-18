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

void ServoMotorPWM::init() {
  device_start();
}

Status ServoMotorPWM::device_start() {
  uint32_t counter_max = 65535 / settings.n_multiplayer; // Max counter value for 16-bit timer
  uint32_t prescaler   = HAL_RCC_GetSysClockFreq() / (settings.pwm_frequency * counter_max) - 1;

  if(prescaler < 0 || prescaler > 65535) {
    return Status::Invalid("ServoMotorPWM: Invalid prescaler value (must be between 0 and 65535)");
  }

  float cpr = (settings.max_angle_rad - settings.min_angle_rad) /
              ((settings.max_pulse_width_us - settings.min_pulse_width_us) / 1000000.0f);
  if(cpr <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid cpr (must be > 0)");
  }

  count_per_rad = cpr;
  min_pulse_width =
  static_cast<uint32_t>((float)(settings.min_pulse_width_us / 1000000.0f * settings.pwm_frequency * counter_max));

  __HAL_TIM_SET_PRESCALER(&htim, prescaler);    // Set the prescaler in the timer handle
  __HAL_TIM_SET_AUTORELOAD(&htim, counter_max); // Set the auto-reload value in the timer handle
  set_enable(false);

  // Start the PWM signal generation
  if(HAL_TIM_PWM_Start(&htim, static_cast<uint32_t>(timer_channel)) != HAL_OK) {
    status = Status::HalError("Failed to start PWM");
  } else {
    status = Status::OK();
  }
}

void ServoMotorPWM::set_velocity(float speed) {
  // Servo motors are not speed controlled, so this function does nothing
}

void ServoMotorPWM::set_torque(float torque) {
  // Servo motors are not torque controlled, so this function does nothing
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
  // Servo motors do not have a velocity setting, so this function does nothing
}


void ServoMotorPWM::set_min_velocity(float min_velocity) {
  // Servo motors do not have a velocity setting, so this function does nothing
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
  const auto *servo_settings = static_cast<const ServoMotorPWMSettings *>(&_settings);
  settings                   = *servo_settings;
  if(settings.pwm_frequency <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid pwm_frequency (must be > 0)");
  }
  if(settings.min_pulse_width_us <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid min_pulse_width_us (must be > 0)");
  }
  if(settings.max_pulse_width_us <= 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid max_pulse_width_us (must be > 0)");
  }
  if(settings.min_angle_rad < 0.0f) {
    return Status::Invalid("ServoMotorPWM: Invalid min_angle_rad (must be >= 0)");
  }
  if(settings.max_angle_rad <= settings.min_angle_rad) {
    return Status::Invalid("ServoMotorPWM: max_angle_rad must be greater than min_angle_rad");
  }
  if(settings.n_multiplayer < 1) {
    return Status::Invalid("ServoMotorPWM: n_multiplayer must be >= 1");
  }
  return Status::OK();
}