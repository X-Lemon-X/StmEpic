#pragma once

#include "motor.hpp"
#include <can.hpp>
#include <movement_controler.hpp>

namespace stmepic::motor {
struct VescMotorSettings : DeviceSettings {
  uint32_t base_address;
  float gear_ratio;
  float current_to_torque;
  uint32_t polar_pairs;
};

struct VescParams {
  double current;
  double erpm;
  double duty_cycle;
  double amd_hours;
  double amd_hours_charged;
  double watt_hours;
  double watt_hours_charged;
  double temperature_mosfet;
  double temperature_motor;
  double current_in;
  double pid_pos;
  double voltage;
  double adc1;
  double adc2;
  double adc3;
  double ppm;
};

class VescMotor : public MotorBase, public DeviceThreadedBase {
public:
  static Result<std::shared_ptr<VescMotor>> Make(std::shared_ptr<CanBase> can);
  VescMotor &operator=(const VescMotor &) = delete;

  [[nodiscard]] float get_velocity() const override;
  [[nodiscard]] float get_torque() const override;
  [[nodiscard]] float get_position() const override;
  [[nodiscard]] float get_absolute_position() const override;
  [[nodiscard]] float get_gear_ratio() const override;
  [[nodiscard]] const VescParams &get_vesc_params() const;

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
  // Status device_reset() override;
  // Status device_start() override;
  // Status device_stop() override;
  Status device_set_settings(const DeviceSettings &settings) override;

private:
  VescMotor(std::shared_ptr<CanBase> can);

  Status do_device_task_start() override;
  Status do_device_task_stop() override;
  Status do_device_task_reset() override;

  Status init();
  Status stop();

  static Status task_before(SimpleTask &handler, void *arg);
  static Status task(SimpleTask &handler, void *arg);
  Status handle();

  std::shared_ptr<CanBase> can;

  std::unique_ptr<VescMotorSettings> settings;

  movement::MovementControlMode control_mode;

  float steps_per_revolution;
  // float gear_ratio;
  float max_velocity;
  float min_velocity;
  bool reverse;
  bool enabled;
  Status status;

  movement::MovementState current_state;
  VescParams vesc_params;

  static void can_callback_status_1(CanBase &can, CanDataFrame &msg, void *args);
  static void can_callback_status_2(CanBase &can, CanDataFrame &msg, void *args);
  static void can_callback_status_3(CanBase &can, CanDataFrame &msg, void *args);
  static void can_callback_status_4(CanBase &can, CanDataFrame &msg, void *args);
  static void can_callback_status_5(CanBase &can, CanDataFrame &msg, void *args);
  static void can_callback_status_6(CanBase &can, CanDataFrame &msg, void *args);

  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_1_FRAME_ID = 0x914u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_2_FRAME_ID = 0xE14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_3_FRAME_ID = 0xF14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_4_FRAME_ID = 0x1014u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_5_FRAME_ID = 0x1B14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_6_FRAME_ID = 0x1C14u;

  struct can_vesc_fleft_status_1_t {
    int32_t erpm;
    int16_t current;
    int16_t duty;
  };

  struct can_vesc_fleft_status_2_t {
    int32_t amp_hours;
    int32_t amp_hours_chg;
  };

  struct can_vesc_fleft_status_3_t {
    int32_t wat_hours;
    int32_t wat_hours_chg;
  };

  struct can_vesc_fleft_status_4_t {
    int16_t temp_mosfet;
    int16_t temp_motor;
    int16_t current_in;
    int16_t pid_pos;
  };

  struct can_vesc_fleft_status_5_t {
    int32_t tachometer;
    int16_t volts_in;
  };

  struct can_vesc_fleft_status_6_t {
    int16_t adc1;
    int16_t adc2;
    int16_t adc3;
    int16_t ppm;
  };

  static uint16_t VescMotor::unpack_left_shift_u16(uint8_t value, uint8_t shift, uint8_t mask);
  static uint32_t VescMotor::unpack_left_shift_u32(uint8_t value, uint8_t shift, uint8_t mask);
  static uint16_t VescMotor::unpack_right_shift_u16(uint8_t value, uint8_t shift, uint8_t mask);
  static uint32_t VescMotor::unpack_right_shift_u32(uint8_t value, uint8_t shift, uint8_t mask);

  static int can_vesc_fleft_status_1_unpack(struct can_vesc_fleft_status_1_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_2_unpack(struct can_vesc_fleft_status_2_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_3_unpack(struct can_vesc_fleft_status_3_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_4_unpack(struct can_vesc_fleft_status_4_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_5_unpack(struct can_vesc_fleft_status_5_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_6_unpack(struct can_vesc_fleft_status_6_t *dst_p, const uint8_t *src_p, size_t size);
};

} // namespace stmepic::motor