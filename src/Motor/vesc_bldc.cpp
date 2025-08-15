#include "vesc_bldc.hpp"
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
  return settings.gear_ratio;
}

const VescParams &VescMotor::get_vesc_params() const {
  return vesc_params;
}

void VescMotor::set_velocity(const float speed) {
  control_mode           = movement::MovementControlMode::VELOCITY;
  current_state.velocity = speed;
}

void VescMotor::set_torque(const float torque) {
  control_mode         = movement::MovementControlMode::TORQUE;
  current_state.torque = torque;
}

void VescMotor::set_position(const float position) {
  control_mode           = movement::MovementControlMode::POSITION;
  current_state.position = position;
}

void VescMotor::set_enable(const bool enable) {
  if(!enable) {
    current_state.velocity = 0;
  }
  enabled = enable;
}

void VescMotor::set_gear_ratio(const float gear_ratio) {
  settings.gear_ratio = gear_ratio;
}

void VescMotor::set_max_velocity(const float max_velocity) {
  // control_mode       = movement::MovementControlMode::VELOCITY; te chyba nie powinno zmieniać trybu kontrolnego   
  this->max_velocity = max_velocity;
}

void VescMotor::set_min_velocity(const float min_velocity) {
  // control_mode       = movement::MovementControlMode::VELOCITY; // to samo co wyżej
  this->min_velocity = min_velocity;
}

void VescMotor::set_reverse(const bool reverse) {
  this->reverse = reverse;
}

bool VescMotor::device_ok() {
  // ... 
  // ja pierdole ???
  // co tu się stało?
  //
  // Status status = device_get_status();
  // if(!status.ok())  
  //   return false;
  // return status.ok();
  return status.ok();
}

Result<bool> VescMotor::device_is_connected() {
  // to akurat jesteś w w stanie sprawdzić czy status jest obierany z jakims timeoutem 
  // więc to poprawki
  return Result<bool>::OK(true);
}

Status VescMotor::device_get_status() {
  return status;
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
  if(vesc_settings->polar_pairs == 0)  // jak nie chcesz żeby ktos  dawał tylko naturalne liczby to użyj unsigned 
    return Status::Invalid("VescMotor: Invalid polar_pairs cant be 0");
  this->settings = *vesc_settings;
  return Status::OK();
}

Status VescMotor::init() {
  const uint32_t st1_id = (CAN_VESC_FLEFT_STATUS_1_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st2_id = (CAN_VESC_FLEFT_STATUS_2_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st3_id = (CAN_VESC_FLEFT_STATUS_3_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st4_id = (CAN_VESC_FLEFT_STATUS_4_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st5_id = (CAN_VESC_FLEFT_STATUS_5_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st6_id = (CAN_VESC_FLEFT_STATUS_6_FRAME_ID & 0xffffff00) | settings.base_address;
  can->add_callback(st1_id, can_callback_status_1, this);
  can->add_callback(st2_id, can_callback_status_2, this);
  can->add_callback(st3_id, can_callback_status_3, this);
  can->add_callback(st4_id, can_callback_status_4, this);
  can->add_callback(st5_id, can_callback_status_5, this);
  can->add_callback(st6_id, can_callback_status_6, this);
  return Status::OK();
}

Status VescMotor::stop() {
  const uint32_t st1_id = (CAN_VESC_FLEFT_STATUS_1_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st2_id = (CAN_VESC_FLEFT_STATUS_2_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st3_id = (CAN_VESC_FLEFT_STATUS_3_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st4_id = (CAN_VESC_FLEFT_STATUS_4_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st5_id = (CAN_VESC_FLEFT_STATUS_5_FRAME_ID & 0xffffff00) | settings.base_address;
  const uint32_t st6_id = (CAN_VESC_FLEFT_STATUS_6_FRAME_ID & 0xffffff00) | settings.base_address;
  can->remove_callback(st1_id);
  can->remove_callback(st2_id);
  can->remove_callback(st3_id);
  can->remove_callback(st4_id);
  can->remove_callback(st5_id);
  can->remove_callback(st6_id);
  return Status::OK();
}

Status VescMotor::do_device_task_reset() {
  stop();
  // STMEPIC_RETURN_ON_ERROR(device_stop()); // to sprawi że wpadnie w infinite rekursywnego loopa 
  init();
  return device_start();
}

Status VescMotor::do_device_task_start() {
  return do_default_task_start(task, task_before, this);
}

Status VescMotor::do_device_task_stop() {
  return do_default_task_stop();
}

Status VescMotor::task_before(SimpleTask &handler, void *arg) {
  (void)handler;
  auto *motor = static_cast<VescMotor *>(arg);
  motor->init();
  return motor->device_start();
}

Status VescMotor::task(SimpleTask &handler, void *arg) {
  (void)handler;
  auto *motor = static_cast<VescMotor *>(arg);
  return motor->handle();
}

Status VescMotor::handle() {
  CanDataFrame frame;
  if(enabled)
    switch(control_mode) {
    case movement::MovementControlMode::VELOCITY: {
      can_vesc_fleft_set_rpm_t str;
      double rpm = current_state.velocity * 60.0 / (2.0 * M_PI) * settings.gear_ratio * settings.polar_pairs;
      str.rpm    = (int32_t)rpm;
      frame.frame_id    = (CAN_VESC_FLEFT_SET_RPM_FRAME_ID & 0xffffff00) | settings.base_address;
      frame.data_size   = CAN_VESC_FLEFT_SET_RPM_LENGTH;
      frame.extended_id = CAN_VESC_FLEFT_SET_RPM_IS_EXTENDED;
      can_vesc_fleft_set_rpm_pack(frame.data, &str, frame.data_size);
      break;
    }
    case movement::MovementControlMode::POSITION: {
      can_vesc_fleft_set_pos_t str;
      double pos        = current_state.position * 1000.0 * settings.gear_ratio;
      str.position      = (int32_t)pos;
      frame.frame_id    = (CAN_VESC_FLEFT_SET_POS_FRAME_ID & 0xffffff00) | settings.base_address;
      frame.data_size   = CAN_VESC_FLEFT_SET_POS_LENGTH;
      frame.extended_id = CAN_VESC_FLEFT_SET_POS_IS_EXTENDED;
      can_vesc_fleft_set_pos_pack(frame.data, &str, frame.data_size);
      break;
    }
    case movement::MovementControlMode::TORQUE: {
      can_vesc_fleft_set_current_t str;
      double current    = current_state.torque / settings.current_to_torque * 1000.0 / settings.gear_ratio;
      str.current       = (int32_t)current;
      frame.frame_id    = (CAN_VESC_FLEFT_SET_CURRENT_FRAME_ID & 0xffffff00) | settings.base_address;
      frame.data_size   = CAN_VESC_FLEFT_SET_CURRENT_LENGTH;
      frame.extended_id = CAN_VESC_FLEFT_SET_CURRENT_IS_EXTENDED;
      can_vesc_fleft_set_current_pack(frame.data, &str, frame.data_size);
      break;
    }
    default: status = Status::ExecutionError("Unknown control mode"); return status;
    }

  return Status::OK();
}

void VescMotor::can_callback_status_1(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_1_t status;

  if(can_vesc_fleft_status_1_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 1");
    return;
  }

  motor->vesc_params.current    = static_cast<float>(status.current);
  motor->vesc_params.duty_cycle = static_cast<float>(status.duty);
  motor->vesc_params.erpm = static_cast<float>(status.erpm) * motor->settings.gear_ratio * motor->settings.polar_pairs;

  motor->current_state.velocity = (static_cast<float>(status.erpm) / 60.0f) * (2.0f * static_cast<float>(M_PI)) /
                                  (motor->settings.gear_ratio * motor->settings.polar_pairs);
}

void VescMotor::can_callback_status_2(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_2_t status;

  if(can_vesc_fleft_status_2_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 2");
    return;
  }
  motor->vesc_params.amd_hours         = (double)status.amp_hours * 1000;
  motor->vesc_params.amd_hours_charged = (double)status.amp_hours_chg * 1000;
}

void VescMotor::can_callback_status_3(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_3_t status;

  if(can_vesc_fleft_status_3_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 3");
    return;
  }
  motor->vesc_params.watt_hours         = (double)status.wat_hours * 1000;
  motor->vesc_params.watt_hours_charged = (double)status.wat_hours_chg * 1000;
}

void VescMotor::can_callback_status_4(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_4_t status;

  if(can_vesc_fleft_status_4_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 4");
    return;
  }
  motor->vesc_params.current_in         = (double)status.current_in;
  motor->vesc_params.pid_pos            = (double)status.pid_pos;
  motor->vesc_params.temperature_mosfet = (double)status.temp_mosfet;
  motor->vesc_params.temperature_motor  = (double)status.temp_motor;
}

void VescMotor::can_callback_status_5(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_5_t status;

  if(can_vesc_fleft_status_5_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 5");
    return;
  }
  double scale_for_tachometer   = 4.0 * M_PI / 360.0; //  2 * 2 * M_PI = 360 deg
  motor->current_state.position = (double)status.tachometer * scale_for_tachometer;
  motor->vesc_params.voltage    = (double)status.volts_in * 0.1;
}

void VescMotor::can_callback_status_6(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  auto *motor = static_cast<VescMotor *>(args);
  can_vesc_fleft_status_6_t status;

  if(can_vesc_fleft_status_6_unpack(&status, msg.data, msg.data_size)) {
    motor->status = Status::ExecutionError("Failed to unpack VESC status 6");
    return;
  }
  motor->vesc_params.adc1 = (double)status.adc1 * 1000;
  motor->vesc_params.adc2 = (double)status.adc2 * 1000;
  motor->vesc_params.adc3 = (double)status.adc3 * 1000;
  motor->vesc_params.ppm  = (double)status.ppm * 1000;
}

uint16_t VescMotor::unpack_left_shift_u16(uint8_t value, uint8_t shift, uint8_t mask) {
  return (uint16_t)((uint16_t)(value & mask) << shift);
}

uint32_t VescMotor::unpack_left_shift_u32(uint8_t value, uint8_t shift, uint8_t mask) {
  return ((uint32_t)(value & mask) << shift);
}

uint16_t VescMotor::unpack_right_shift_u16(uint8_t value, uint8_t shift, uint8_t mask) {
  return (uint16_t)((uint16_t)(value & mask) >> shift);
}

uint32_t VescMotor::unpack_right_shift_u32(uint8_t value, uint8_t shift, uint8_t mask) {
  return ((uint32_t)(value & mask) >> shift);
}

uint8_t VescMotor::pack_left_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
  return (uint8_t)((uint8_t)(value << shift) & mask);
}

uint8_t VescMotor::pack_right_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
  return (uint8_t)((uint8_t)(value >> shift) & mask);
}

int VescMotor::can_vesc_fleft_status_1_unpack(struct can_vesc_fleft_status_1_t *dst_p, const uint8_t *src_p, size_t size) {
  uint16_t current;
  uint16_t duty;
  uint32_t erpm;

  if(size < 8u) {
    return (-EINVAL);
  }

  erpm = unpack_left_shift_u32(src_p[0], 24u, 0xffu);
  erpm |= unpack_left_shift_u32(src_p[1], 16u, 0xffu);
  erpm |= unpack_left_shift_u32(src_p[2], 8u, 0xffu);
  erpm |= unpack_right_shift_u32(src_p[3], 0u, 0xffu);
  dst_p->erpm = (int32_t)erpm;
  current     = unpack_left_shift_u16(src_p[4], 8u, 0xffu);
  current |= unpack_right_shift_u16(src_p[5], 0u, 0xffu);
  dst_p->current = (int16_t)current;
  duty           = unpack_left_shift_u16(src_p[6], 8u, 0xffu);
  duty |= unpack_right_shift_u16(src_p[7], 0u, 0xffu);
  dst_p->duty = (int16_t)duty;

  return (0);
}

int VescMotor::can_vesc_fleft_status_2_unpack(struct can_vesc_fleft_status_2_t *dst_p, const uint8_t *src_p, size_t size) {
  uint32_t amp_hours;
  uint32_t amp_hours_chg;

  if(size < 8u) {
    return (-EINVAL);
  }

  amp_hours = unpack_left_shift_u32(src_p[0], 24u, 0xffu);
  amp_hours |= unpack_left_shift_u32(src_p[1], 16u, 0xffu);
  amp_hours |= unpack_left_shift_u32(src_p[2], 8u, 0xffu);
  amp_hours |= unpack_right_shift_u32(src_p[3], 0u, 0xffu);
  dst_p->amp_hours = (int32_t)amp_hours;
  amp_hours_chg    = unpack_left_shift_u32(src_p[4], 24u, 0xffu);
  amp_hours_chg |= unpack_left_shift_u32(src_p[5], 16u, 0xffu);
  amp_hours_chg |= unpack_left_shift_u32(src_p[6], 8u, 0xffu);
  amp_hours_chg |= unpack_right_shift_u32(src_p[7], 0u, 0xffu);
  dst_p->amp_hours_chg = (int32_t)amp_hours_chg;

  return (0);
}

int VescMotor::can_vesc_fleft_status_3_unpack(struct can_vesc_fleft_status_3_t *dst_p, const uint8_t *src_p, size_t size) {
  uint32_t wat_hours;
  uint32_t wat_hours_chg;

  if(size < 8u) {
    return (-EINVAL);
  }

  wat_hours = unpack_left_shift_u32(src_p[0], 24u, 0xffu);
  wat_hours |= unpack_left_shift_u32(src_p[1], 16u, 0xffu);
  wat_hours |= unpack_left_shift_u32(src_p[2], 8u, 0xffu);
  wat_hours |= unpack_right_shift_u32(src_p[3], 0u, 0xffu);
  dst_p->wat_hours = (int32_t)wat_hours;
  wat_hours_chg    = unpack_left_shift_u32(src_p[4], 24u, 0xffu);
  wat_hours_chg |= unpack_left_shift_u32(src_p[5], 16u, 0xffu);
  wat_hours_chg |= unpack_left_shift_u32(src_p[6], 8u, 0xffu);
  wat_hours_chg |= unpack_right_shift_u32(src_p[7], 0u, 0xffu);
  dst_p->wat_hours_chg = (int32_t)wat_hours_chg;

  return (0);
}

int VescMotor::can_vesc_fleft_status_4_unpack(struct can_vesc_fleft_status_4_t *dst_p, const uint8_t *src_p, size_t size) {
  uint16_t current_in;
  uint16_t pid_pos;
  uint16_t temp_mosfet;
  uint16_t temp_motor;

  if(size < 8u) {
    return (-EINVAL);
  }

  temp_mosfet = unpack_left_shift_u16(src_p[0], 8u, 0xffu);
  temp_mosfet |= unpack_right_shift_u16(src_p[1], 0u, 0xffu);
  dst_p->temp_mosfet = (int16_t)temp_mosfet;
  temp_motor         = unpack_left_shift_u16(src_p[2], 8u, 0xffu);
  temp_motor |= unpack_right_shift_u16(src_p[3], 0u, 0xffu);
  dst_p->temp_motor = (int16_t)temp_motor;
  current_in        = unpack_left_shift_u16(src_p[4], 8u, 0xffu);
  current_in |= unpack_right_shift_u16(src_p[5], 0u, 0xffu);
  dst_p->current_in = (int16_t)current_in;
  pid_pos           = unpack_left_shift_u16(src_p[6], 8u, 0xffu);
  pid_pos |= unpack_right_shift_u16(src_p[7], 0u, 0xffu);
  dst_p->pid_pos = (int16_t)pid_pos;

  return (0);
}

int VescMotor::can_vesc_fleft_status_5_unpack(struct can_vesc_fleft_status_5_t *dst_p, const uint8_t *src_p, size_t size) {
  uint16_t volts_in;
  uint32_t tachometer;

  if(size < 6u) {
    return (-EINVAL);
  }

  tachometer = unpack_left_shift_u32(src_p[0], 24u, 0xffu);
  tachometer |= unpack_left_shift_u32(src_p[1], 16u, 0xffu);
  tachometer |= unpack_left_shift_u32(src_p[2], 8u, 0xffu);
  tachometer |= unpack_right_shift_u32(src_p[3], 0u, 0xffu);
  dst_p->tachometer = (int32_t)tachometer;
  volts_in          = unpack_left_shift_u16(src_p[4], 8u, 0xffu);
  volts_in |= unpack_right_shift_u16(src_p[5], 0u, 0xffu);
  dst_p->volts_in = (int16_t)volts_in;

  return (0);
}

int VescMotor::can_vesc_fleft_status_6_unpack(struct can_vesc_fleft_status_6_t *dst_p, const uint8_t *src_p, size_t size) {
  uint16_t adc1;
  uint16_t adc2;
  uint16_t adc3;
  uint16_t ppm;

  if(size < 8u) {
    return (-EINVAL);
  }

  adc1 = unpack_left_shift_u16(src_p[0], 8u, 0xffu);
  adc1 |= unpack_right_shift_u16(src_p[1], 0u, 0xffu);
  dst_p->adc1 = (int16_t)adc1;
  adc2        = unpack_left_shift_u16(src_p[2], 8u, 0xffu);
  adc2 |= unpack_right_shift_u16(src_p[3], 0u, 0xffu);
  dst_p->adc2 = (int16_t)adc2;
  adc3        = unpack_left_shift_u16(src_p[4], 8u, 0xffu);
  adc3 |= unpack_right_shift_u16(src_p[5], 0u, 0xffu);
  dst_p->adc3 = (int16_t)adc3;
  ppm         = unpack_left_shift_u16(src_p[6], 8u, 0xffu);
  ppm |= unpack_right_shift_u16(src_p[7], 0u, 0xffu);
  dst_p->ppm = (int16_t)ppm;

  return (0);
}

int VescMotor::can_vesc_fleft_set_rpm_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_rpm_t *src_p, size_t size) {
  uint32_t rpm;

  if(size < 4u) {
    return (-EINVAL);
  }

  memset(&dst_p[0], 0, 4);

  rpm = (uint32_t)src_p->rpm;
  dst_p[0] |= pack_right_shift_u32(rpm, 24u, 0xffu);
  dst_p[1] |= pack_right_shift_u32(rpm, 16u, 0xffu);
  dst_p[2] |= pack_right_shift_u32(rpm, 8u, 0xffu);
  dst_p[3] |= pack_left_shift_u32(rpm, 0u, 0xffu);

  return (4);
}

int VescMotor::can_vesc_fleft_set_pos_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_pos_t *src_p, size_t size) {
  uint32_t position;

  if(size < 4u) {
    return (-EINVAL);
  }

  memset(&dst_p[0], 0, 4);

  position = (uint32_t)src_p->position;
  dst_p[0] |= pack_right_shift_u32(position, 24u, 0xffu);
  dst_p[1] |= pack_right_shift_u32(position, 16u, 0xffu);
  dst_p[2] |= pack_right_shift_u32(position, 8u, 0xffu);
  dst_p[3] |= pack_left_shift_u32(position, 0u, 0xffu);

  return (4);
}

int VescMotor::can_vesc_fleft_set_current_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_current_t *src_p, size_t size) {
  uint32_t current;

  if(size < 4u) {
    return (-EINVAL);
  }

  memset(&dst_p[0], 0, 4);

  current = (uint32_t)src_p->current;
  dst_p[0] |= pack_right_shift_u32(current, 24u, 0xffu);
  dst_p[1] |= pack_right_shift_u32(current, 16u, 0xffu);
  dst_p[2] |= pack_right_shift_u32(current, 8u, 0xffu);
  dst_p[3] |= pack_left_shift_u32(current, 0u, 0xffu);

  return (4);
}