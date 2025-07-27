#pragma once

#include "motor.hpp"
#include <can.hpp>
#include <movement_controler.hpp>

namespace stmepic::motor {

struct VescMotorSettings : DeviceSettings {
  uint32_t base_address{ 0x14 };
  float gear_ratio{ 1.0 };
  float current_to_torque{ 1.0 };
  uint32_t polar_pairs{ 7 };
};

class VescMotor : public MotorBase, public DeviceThreadedBase {
public:
  static Result<std::shared_ptr<VescMotor>> Make(std::shared_ptr<CanBase> can);
  VescMotor &operator=(const VescMotor &) = delete;

  void init() override;
  [[nodiscard]] float get_velocity() const override;
  [[nodiscard]] float get_torque() const override;
  [[nodiscard]] float get_position() const override;
  [[nodiscard]] float get_absolute_position() const override;
  [[nodiscard]] float get_gear_ratio() const override;

  void set_velocity(float speed) override;
  void set_torque(float torque) override;
  void set_position(float position) override;
  void set_enable(bool enable) override;
  void set_gear_ratio(float gear_ratio) override;
  void set_max_velocity(float max_velocity) override;
  void set_min_velocity(float min_velocity) override;
  void set_reverse(bool reverse) override;

  bool device_ok() override;
  Result<bool> device_is_connected() override;
  Status device_get_status() override;
  Status device_reset() override;
  Status device_start() override;
  Status device_stop() override;
  Status device_set_settings(const DeviceSettings &settings) override;

private:
  VescMotor(const std::shared_ptr<CanBase> can);

  Status do_device_task_start() override;
  Status do_device_task_stop() override;

  static Status task_before(SimpleTask &handler, void *arg);
  static Status task(SimpleTask &handler, void *arg);
  Status handle();

  std::shared_ptr<CanBase> can;
  uint32_t base_address;

  std::unique_ptr<VescMotorSettings> settings;

  float steps_per_revolution;
  // float gear_ratio;
  float max_velocity;
  float min_velocity;
  bool reverse;
  bool enable_reversed;

  movement::MovementState current_state;
};

} // namespace stmepic::motor