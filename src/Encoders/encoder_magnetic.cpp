
#include "encoder_magnetic.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include "simple_task.hpp"
#include <cmath>
#include <cstdint>


using namespace stmepic::encoders;
using namespace stmepic;


#define PI_m2 6.28318530717958647692f


EncoderAbsoluteMagnetic::EncoderAbsoluteMagnetic(std::shared_ptr<I2C> _hi2c,
                                                 uint32_t _resolution,
                                                 std::shared_ptr<filters::FilterBase> _filter_angle,
                                                 std::shared_ptr<filters::FilterBase> _filter_velocity)

: hi2c(_hi2c), encoder_connected(false), device_status(Status::OK()), filter_angle(_filter_angle),
  filter_velocity(_filter_velocity), last_time(stmepic::Ticker::get_instance().get_seconds()), prev_angle(0),
  current_angle(0), current_velocity(0), prev_angle_rad_raw(0), prev_angle_velocity(0), over_drive_angle(0),
  absolute_angle(0), ratio(1), offset(0), dead_zone_correction_angle(0), reverse(false), resolution(_resolution) {
}


void EncoderAbsoluteMagnetic::init() {
  float angle = read_angle_rads();

  // correct the angle begin value to avoid false rotation dircetion
  // after first starting after power down
  if(dead_zone_correction_angle != 0)
    this->over_drive_angle = angle > this->dead_zone_correction_angle ? -PI_m2 : 0;
  else // if no dead zone correction is needed then coret to the shortest path to 0
    this->over_drive_angle = std::abs(PI_m2 - angle) < std::abs(angle) ? -PI_m2 : 0;

  this->prev_angle = angle;
  read_angle();
  this->prev_angle_velocity = this->absolute_angle;

  if(this->filter_angle != nullptr)
    filter_angle->set_init_value(read_angle());
}

float EncoderAbsoluteMagnetic::calculate_velocity(float angle) {
  float current_tiem     = stmepic::Ticker::get_instance().get_seconds();
  const float dt         = current_tiem - last_time;
  float current_velocity = (angle - prev_angle_velocity) / dt;
  last_time              = current_tiem;
  if(this->filter_velocity != nullptr)
    current_velocity = filter_velocity->calculate(current_velocity);
  prev_angle_velocity = angle;
  return current_velocity;
}

float EncoderAbsoluteMagnetic::read_angle_rads() {
  auto maybe_reg = read_raw_angle();
  if(!maybe_reg.ok())
    return prev_angle_rad_raw;
  float angle = (float)maybe_reg.valueOrDie() * PI_m2 / (float)this->resolution;
  if(this->reverse)
    angle = PI_m2 - angle;
  angle += this->offset;
  if(angle > PI_m2)
    angle -= PI_m2;
  if(angle < 0)
    angle += PI_m2;
  prev_angle_rad_raw = angle;
  return angle;
}

float EncoderAbsoluteMagnetic::read_angle() {
  float angle = read_angle_rads();

  if(prev_angle - angle > ANGLE_MAX_DEFFERENCE)
    over_drive_angle += PI_m2;
  else if(angle - prev_angle > ANGLE_MAX_DEFFERENCE)
    over_drive_angle -= PI_m2;
  prev_angle = angle;

  if(this->filter_angle != nullptr)
    angle = filter_angle->calculate(angle);

  absolute_angle = (angle + over_drive_angle) * ratio;
  int rotation   = (int)(absolute_angle / PI_m2);
  current_angle  = (float)(absolute_angle - rotation * PI_m2);


  current_velocity = calculate_velocity(absolute_angle);

  return angle;
}

void EncoderAbsoluteMagnetic::handle() {
  read_angle();
}

float EncoderAbsoluteMagnetic::get_velocity() const {
  return this->current_velocity;
}

float EncoderAbsoluteMagnetic::get_torque() const {
  return 0;
}

float EncoderAbsoluteMagnetic::get_angle() const {
  return this->current_angle;
}

float EncoderAbsoluteMagnetic::get_absoulute_angle() const {
  return this->absolute_angle;
}

void EncoderAbsoluteMagnetic::set_offset(float offset) {
  this->offset = offset;
}

void EncoderAbsoluteMagnetic::set_reverse(bool reverse) {
  this->reverse = reverse;
}


void EncoderAbsoluteMagnetic::set_dead_zone_correction_angle(float dead_zone_correction_angle) {
  this->dead_zone_correction_angle = std::abs(dead_zone_correction_angle);
}

void EncoderAbsoluteMagnetic::set_ratio(float ratio) {
  this->ratio = ratio;
}

bool EncoderAbsoluteMagnetic::device_ok() {
  return device_status.ok();
}

stmepic::Result<bool> EncoderAbsoluteMagnetic::device_is_connected() {
  return Result<bool>::OK(encoder_connected);
}

stmepic::Status EncoderAbsoluteMagnetic::device_get_status() {
  if(hi2c == nullptr)
    return Status::Invalid("I2C is not initialized");
  return device_status;
}

stmepic::Status EncoderAbsoluteMagnetic::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  return device_start();
}

stmepic::Status EncoderAbsoluteMagnetic::device_start() {
  return device_get_status().status();
}


stmepic::Status EncoderAbsoluteMagnetic::device_stop() {
  return do_default_task_stop();
}

stmepic::Status EncoderAbsoluteMagnetic::do_device_task_start() {
  return DeviceThreadedBase::do_default_task_start(task_encoder, task_encoder_before, this);
}

stmepic::Status EncoderAbsoluteMagnetic::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}

void EncoderAbsoluteMagnetic::task_encoder_before(SimpleTask &handler, void *arg) {
  (void)handler;
  EncoderAbsoluteMagnetic *encoder = static_cast<EncoderAbsoluteMagnetic *>(arg);
  encoder->init();
}

void EncoderAbsoluteMagnetic::task_encoder(SimpleTask &handler, void *arg) {
  (void)handler;
  EncoderAbsoluteMagnetic *encoder = static_cast<EncoderAbsoluteMagnetic *>(arg);
  encoder->handle();
}


//********************************************************************************************************************
// AS5600 Encoder

EncoderAbsoluteMagneticAS5600::EncoderAbsoluteMagneticAS5600(std::shared_ptr<I2C> hi2c,
                                                             uint16_t _address,
                                                             uint32_t resolution,
                                                             std::shared_ptr<filters::FilterBase> filter_angle,
                                                             std::shared_ptr<filters::FilterBase> filter_velocity)
: EncoderAbsoluteMagnetic(hi2c, resolution, filter_angle, filter_velocity), address(_address) {
}

Result<std::shared_ptr<EncoderAbsoluteMagneticAS5600>>
EncoderAbsoluteMagneticAS5600::Make(std::shared_ptr<I2C> hi2c,
                                    encoder_AS5600_addresses _address,
                                    std::shared_ptr<filters::FilterBase> filter_angle,
                                    std::shared_ptr<filters::FilterBase> filter_velocity) {
  if(hi2c == nullptr)
    return Status::Invalid("I2C is not nullptr");
  auto encoder = std::shared_ptr<EncoderAbsoluteMagneticAS5600>(
  new EncoderAbsoluteMagneticAS5600(hi2c, (uint16_t)_address, 4096, filter_angle, filter_velocity));
  return Result<decltype(encoder)>::OK(encoder);
}

Result<uint32_t> EncoderAbsoluteMagneticAS5600::read_raw_angle() {
  uint8_t data[2];
  auto status       = hi2c->read(address, 0x0C, data, 2);
  encoder_connected = status.ok() || status.status_code() != StatusCode::TimeOut;
  device_status     = status;
  STMEPIC_RETURN_ON_ERROR(status);
  uint32_t reg = (uint32_t)(data[0] & 0x0F) << 8;
  reg |= (uint32_t)(data[1]);
  return Result<uint32_t>::OK(reg);
}

//********************************************************************************************************************
// Magnatek MT6701 Encoder

EncoderAbsoluteMagneticMT6701::EncoderAbsoluteMagneticMT6701(std::shared_ptr<I2C> hi2c,
                                                             uint16_t _address,
                                                             uint32_t resolution,
                                                             std::shared_ptr<filters::FilterBase> filter_angle,
                                                             std::shared_ptr<filters::FilterBase> filter_velocity)
: EncoderAbsoluteMagnetic(hi2c, resolution, filter_angle, filter_velocity), address(_address) {
}
Result<std::shared_ptr<EncoderAbsoluteMagneticMT6701>>
EncoderAbsoluteMagneticMT6701::Make(std::shared_ptr<I2C> hi2c,
                                    encoder_MT6701_addresses address,
                                    std::shared_ptr<filters::FilterBase> filter_angle,
                                    std::shared_ptr<filters::FilterBase> filter_velocity) {
  if(hi2c == nullptr)
    return Status::Invalid("I2C is not initialized");
  auto encoder = std::shared_ptr<EncoderAbsoluteMagneticMT6701>(
  new EncoderAbsoluteMagneticMT6701(hi2c, (uint16_t)address, 16384, filter_angle, filter_velocity));
  return Result<decltype(encoder)>::OK(encoder);
}

Result<uint32_t> EncoderAbsoluteMagneticMT6701::read_raw_angle() {
  uint8_t data[2];
  auto status       = hi2c->read(address, 0x03, data, 2);
  encoder_connected = status.ok() || status.status_code() != StatusCode::TimeOut;
  device_status     = status;
  STMEPIC_RETURN_ON_ERROR(status);
  uint32_t reg = (uint32_t)data[0] << 6;
  reg |= (uint32_t)(data[1] & 0xfc) >> 2;
  return Result<uint32_t>::OK(reg);
}