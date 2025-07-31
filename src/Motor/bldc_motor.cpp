#include "bldc_motor.hpp"
#include "status.hpp"

using namespace stmepic::motor;
using namespace stmepic;

Result<std::shared_ptr<VescMotor>> VescMotor::Make(const std::shared_ptr<CanBase> can) {
  if(can == nullptr)
    return Status::Invalid("CAN is not nullptr");
  auto res = std::shared_ptr<VescMotor>(new VescMotor(can));
  return Result<std::shared_ptr<VescMotor>>::OK(res);
}

VescMotor::VescMotor(const std::shared_ptr<CanBase> can)
: can(can), steps_per_revolution(400), max_velocity(0), min_velocity(0), reverse(false), enabled(false),
  status(Status::ExecutionError("VescMotor not initialized")) {
  VescMotorSettings s;
  s.base_address      = 0x14;
  s.gear_ratio        = 1.0;
  s.current_to_torque = 0.0665;
  s.polar_pairs       = 7;
  VescMotor::device_set_settings(s);
}

float VescMotor::get_velocity() const {
  return current_state.velocity;
}

float VescMotor::get_torque() const {
  return current_state.torque;
}

float VescMotor::get_position() const {
  return current_state.position;
}

float VescMotor::get_absolute_position() const {
  return current_state.position;
}

float VescMotor::get_gear_ratio() const {
  return settings->gear_ratio;
}

void VescMotor::set_velocity(const float speed) {
  current_state.velocity = speed;
}

void VescMotor::set_torque(const float torque) {
  current_state.torque = torque;
}

void VescMotor::set_position(const float position) {
  current_state.position = position;
}

void VescMotor::set_enable(const bool enable) {
  this->enabled = enable;
}

void VescMotor::set_gear_ratio(const float gear_ratio) {
  settings->gear_ratio = gear_ratio;
}

void VescMotor::set_max_velocity(const float max_velocity) {
  this->max_velocity = max_velocity;
}

void VescMotor::set_min_velocity(const float min_velocity) {
  this->min_velocity = min_velocity;
}

void VescMotor::set_reverse(const bool reverse) {
  this->reverse = reverse;
}

bool VescMotor::device_ok() {
  Status status = device_get_status();
  if(!status.ok())
    return false;
  return status.ok();
}

Result<bool> VescMotor::device_is_connected() {
  return Result<bool>::OK(true);
}

Status VescMotor::device_get_status() {
  return Status::OK();
}

Status VescMotor::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  return device_start();
}

Status VescMotor::device_start() {
  set_enable(true);
  return Status::OK();
}

Status VescMotor::device_stop() {
  set_velocity(0.0f);
  set_enable(false);
  Status stop_status = do_device_task_stop();
  if(!stop_status.ok()) {
    return stop_status;
  }
  return Status::OK();
}

Status VescMotor::device_set_settings(const DeviceSettings &_settings) {
  const auto vesc_settings = dynamic_cast<const VescMotorSettings *>(&_settings);

  if(vesc_settings == nullptr)
    return Status::ExecutionError("Settings are not of type VescMotorSettings");
  if(vesc_settings->base_address <= 0)
    return Status::Invalid("VescMotor: Invalid base_address (must be > 0)");
  if(vesc_settings->current_to_torque <= 0.0f)
    return Status::Invalid("VescMotor: Invalid current_to_torque (must be > 0.0)");
  if(vesc_settings->gear_ratio <= 0.0f)
    return Status::Invalid("VescMotor: Invalid gear_ratio (must be > 0.0)");
  if(vesc_settings->polar_pairs <= 0)
    return Status::Invalid("VescMotor: Invalid polar_pairs (must be > 0)");

  this->settings = std::make_unique<VescMotorSettings>(*vesc_settings);
  return Status::OK();
}

Status VescMotor::do_device_task_start() {
  // wip
  return do_default_task_start(task, task_before, this);
}

Status VescMotor::do_device_task_stop() {
  return do_default_task_stop();
}

Status VescMotor::task_before(SimpleTask &handler, void *arg) {
  (void)handler;
  auto *motor = static_cast<VescMotor *>(arg);
  return motor->device_start();
}

Status VescMotor::task(SimpleTask &handler, void *arg) {
  (void)handler;
  auto *motor = static_cast<VescMotor *>(arg);
  return motor->handle();
}

Status VescMotor::handle() {

  // current_state.position = 123;
  // current_state.velocity = 123;
  // current_state.torque = 123;

  // wip
  // uint8_t data[120] = { 0 };
  // auto a            = huart->read(data, sizeof(data), 3000);
  //
  // if(a == StatusCode::HalBusy) {
  //   huart->hardware_stop();
  //   huart->hardware_start();
  // }
  //
  // log_info("AT Modem" + a.status().to_string() + " data received:" + std::string(reinterpret_cast<const
  // char *>(data))); for(size_t i = 0; i < sizeof(data); ++i) {
  //   if(data[i] == '\0')
  //     continue; // Skip null characters
  //   if(settings->enable_gps) {
  //     nmea_status = nmea_parser.parse_by_character(static_cast<char>(data[i]));
  //   }
  // }
  // auto v = nmea_parser.get_gga_data();
  // log_info("Long: " + std::to_string(v.latitude) + " Lat: " + std::to_string(v.longitude));
  return Status::OK();
}