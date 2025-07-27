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

VescMotor::VescMotor(const std::shared_ptr<CanBase> can) : can(can) {
  steps_per_revolution = 400;
  max_velocity         = 0;
  min_velocity         = 0;
  reverse              = false;
  enable_reversed      = false;
  VescMotorSettings s;
  VescMotor::device_set_settings(s);
  VescMotor::init();
}

void VescMotor::init() {
  VescMotor::device_start();
}

float VescMotor::get_velocity() const {
  // wip
  return current_state.velocity;
}

float VescMotor::get_torque() const {
  // wip
  return current_state.torque;
}

float VescMotor::get_position() const {
  // wip
  return current_state.position;
}

float VescMotor::get_absolute_position() const {
  // wip
}

float VescMotor::get_gear_ratio() const {
  // wip
  return settings->gear_ratio;
}

void VescMotor::set_velocity(const float speed) {
  // wip
  current_state.velocity = speed;
}

void VescMotor::set_torque(const float torque) {
  // wip
  current_state.torque = torque;
}

void VescMotor::set_position(const float position) {
  // wip
  current_state.position = position;
}

void VescMotor::set_enable(const bool enable) {
  // wip
  this->enable_reversed = enable;
}

void VescMotor::set_gear_ratio(const float gear_ratio) {
  // wip
  settings->gear_ratio = gear_ratio;
}

void VescMotor::set_max_velocity(const float max_velocity) {
  // wip
  this->max_velocity = max_velocity;
}

void VescMotor::set_min_velocity(const float min_velocity) {
  // wip
  this->min_velocity = min_velocity;
}

void VescMotor::set_reverse(const bool reverse) {
  // wip
  this->reverse = reverse;
}

bool VescMotor::device_ok() {
  // wip
  return true;
}

Result<bool> VescMotor::device_is_connected() {
  // wip
  return Result<bool>::OK(true);
}

Status VescMotor::device_get_status() {
  // wip
  return Status::OK();
}

Status VescMotor::device_reset() {
  // wip
  return Status::OK();
}

Status VescMotor::device_start() {
  // wip
  // auto core_freq = (float)HAL_RCC_GetHCLKFreq();
  // auto prescaler = (float)htim.Instance->PSC;
  //
  // // equaions to get frequency that will give us desired velocity
  // // base frequency
  // // frequency = velocity * steps_pre_revolutions * gear_ratio
  // this->radians_to_frequency = core_freq / prescaler / ((this->steps_per_revolution * this->gear_ratio) /
  // PIM2); return Status::OK();
}

Status VescMotor::device_stop() {
  // wip
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
  return DeviceThreadedBase::do_default_task_start(task, task_before, this);
}

Status VescMotor::do_device_task_stop() {
  // wip
  return DeviceThreadedBase::do_default_task_stop();
}

Status VescMotor::task_before(SimpleTask &handler, void *arg) {
  // wip
  // (void)handler;
  // VescMotor *bar = static_cast<VescMotor *>(arg);
  // return bar->device_start();
}

Status VescMotor::task(SimpleTask &handler, void *arg) {
  // wip
  // (void)handler;
  // VescMotor *modem = static_cast<VescMotor *>(arg);
  // return modem->handle();
}

Status VescMotor::handle() {
  // wip
  // uint8_t data[120] = { 0 };
  // auto a            = huart->read(data, sizeof(data), 3000);
  //
  // if(a == StatusCode::HalBusy) {
  //   huart->hardware_stop();
  //   huart->hardware_start();
  // }
  //
  // log_info("AT Modem" + a.status().to_string() + " data received:" + std::string(reinterpret_cast<const char *>(data)));
  // for(size_t i = 0; i < sizeof(data); ++i) {
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