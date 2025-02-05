
#include "encoder_magnetic.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cmath>
#include <cstdint>


using namespace stmepic::encoders;
using namespace stmepic;


#define PI_m2 6.28318530717958647692f


uint16_t stmepic::encoders::translate_reg_to_angle_AS5600(uint8_t data1, uint8_t data2) {
  uint16_t reg = (uint16_t)(data1 & 0x0F) << 8;
  reg |= (uint16_t)(data2);
  return reg;
}

uint16_t stmepic::encoders::translate_reg_to_angle_MT6701(uint8_t data1, uint8_t data2) {
  uint16_t reg = (uint16_t)data1 << 6;
  reg |= (uint16_t)(data2 & 0xfc) >> 2;
  return reg;
}

EncoderAbsoluteMagnetic::EncoderAbsoluteMagnetic()
: device_status(Status::Disconnected("Encoder is not connected")) {
  this->resolution                 = 4096;
  this->address                    = 0x36;
  this->reverse                    = false;
  this->offset                     = 0;
  this->data[0]                    = 0;
  this->data[1]                    = 0;
  this->encoder_initiated          = false;
  this->ratio                      = 1;
  this->dead_zone_correction_angle = 0;
  this->reg_to_angle_function      = translate_reg_to_angle_AS5600;
}

void EncoderAbsoluteMagnetic::init(std::shared_ptr<I2C> hi2c,
                                   traslate_reg_to_angle _reg_to_angle_function,
                                   filters::FilterBase *filter_angle,
                                   filters::FilterBase *filter_velocity) {
  this->hi2c                  = hi2c;
  this->filter_angle          = filter_angle;
  this->filter_velocity       = filter_velocity;
  this->last_time             = stmepic::Ticker::get_instance().get_seconds();
  this->current_velocity      = 0;
  this->over_drive_angle      = 0;
  this->reg_to_angle_function = _reg_to_angle_function;
  this->encoder_initiated     = true;
  this->device_status         = Status::OK();
  // bool connected = ping_encoder();

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
}

// bool EncoderAbsoluteMagnetic::ping_encoder(){
//   if(this->encoder_enabled) return false;
//   return HAL_I2C_IsDeviceReady(hi2c, address, 1, 100) == HAL_OK;
// }

stmepic::Result<uint16_t> EncoderAbsoluteMagnetic::read_raw_angle() {
  if(!encoder_initiated)
    return Status::IOError("Encoder is not initiated");

  // HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, address, angle_register, 1, this->data, 2, 5);
  auto status       = hi2c->read(address, angle_register, data, 2);
  encoder_connected = status.ok();
  device_status     = status;
  STMEPIC_RETURN_ON_ERROR(status);
  uint16_t reg = reg_to_angle_function(data[0], data[1]);
  return Result<uint16_t>::OK(reg);
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
  if(!encoder_initiated)
    return 0;
  auto maybe_reg = read_raw_angle();
  if(!maybe_reg.ok())
    return 0;
  float angle = (float)maybe_reg.valueOrDie() * PI_m2 / (float)this->resolution;
  if(this->reverse)
    angle = PI_m2 - angle;
  angle += this->offset;
  if(angle > PI_m2)
    angle -= PI_m2;
  if(angle < 0)
    angle += PI_m2;
  return angle;
}

float EncoderAbsoluteMagnetic::read_angle() {
  if(!encoder_initiated)
    return 0;

  float angle = read_angle_rads();

  if(prev_angle - angle > ANGLE_MAX_DEFFERENCE)
    over_drive_angle += PI_m2;
  else if(angle - prev_angle > ANGLE_MAX_DEFFERENCE)
    over_drive_angle -= PI_m2;
  prev_angle     = angle;
  absolute_angle = (angle + over_drive_angle) * ratio;
  int rotation   = (int)(absolute_angle / PI_m2);
  current_angle  = (float)(absolute_angle - rotation * PI_m2);

  if(this->filter_angle != nullptr)
    absolute_angle = filter_angle->calculate(absolute_angle);
  current_velocity = calculate_velocity(absolute_angle);

  return angle;
}

void EncoderAbsoluteMagnetic::handle() {
  if(!encoder_initiated)
    return;
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

// bool EncoderAbsoluteMagnetic::is_connected() const{
//   return this->encoder_connected;
// }

void EncoderAbsoluteMagnetic::set_resolution(uint16_t resolution) {
  this->resolution = resolution;
}

void EncoderAbsoluteMagnetic::set_offset(float offset) {
  this->offset = offset;
}

void EncoderAbsoluteMagnetic::set_reverse(bool reverse) {
  this->reverse = reverse;
}

void EncoderAbsoluteMagnetic::set_address(uint8_t address) {
  this->address = address;
}

void EncoderAbsoluteMagnetic::set_angle_register(uint8_t angle_register) {
  this->angle_register = angle_register;
}

void EncoderAbsoluteMagnetic::set_magnes_detection_register(uint8_t magnes_detection_register) {
  this->magnes_detection_register = magnes_detection_register;
}

void EncoderAbsoluteMagnetic::set_dead_zone_correction_angle(float dead_zone_correction_angle) {
  this->dead_zone_correction_angle = std::abs(dead_zone_correction_angle);
}

void EncoderAbsoluteMagnetic::set_ratio(float ratio) {
  this->ratio = ratio;
}

bool EncoderAbsoluteMagnetic::device_ok() {
  return encoder_connected;
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
  return DeviceThreadedBase::do_default_task_start(task_encoder, this);
}

stmepic::Status EncoderAbsoluteMagnetic::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}


void EncoderAbsoluteMagnetic::task_encoder(void *arg) {
  EncoderAbsoluteMagnetic *encoder = static_cast<EncoderAbsoluteMagnetic *>(arg);
  encoder->handle();
}