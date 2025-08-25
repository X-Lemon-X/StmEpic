#pragma once

#include "motor.hpp"
#include <can.hpp>
#include <movement_controler.hpp>

/**
 * @file vesc_bldc.hpp
 * @brief Class definition for controlling VESC (Vedder Electronic Speed Controller) BLDC motors.
 *
 */

/**
 * @defgroup Motor
 * @{
 */

/**
 * @defgroup VescMotor
 * @ingroup Motor
 * @brief VescMotor driver
 * @{
 */

/**
 * @brief Settings for configuring a VESC motor.
 */
namespace stmepic::motor {
struct VescMotorSettings : DeviceSettings {
  uint32_t base_address; ///< Base CAN frame address used to calculate the rest of CAN message frame addresses
  float gear_ratio; ///< Gear ratio between motor shaft and output shaft. Used for calculating other attributes in CAN callbacks and the main task.
  float current_to_torque; ///< Conversion factor from motor current to torque (Nm/A).
  uint16_t polar_pairs;    ///< Number of pole pairs in the motor.
};

/**
 * @brief Parameters unique to VESC BLDC motors that aren't present in MotorBase nor in DeviceThreadedBase
 * but also wouldn't be considered settings
 */
struct VescParams {
  float current;
  float erpm;
  float duty_cycle;
  float amp_hours;
  float amp_hours_charged;
  float watt_hours;
  float watt_hours_charged;
  float temperature_mosfet;
  float temperature_motor;
  float current_in;
  float pid_pos;
  float voltage;
  float adc1;
  float adc2;
  float adc3;
  float ppm;
};

/**
 * @class VescMotor
 * @brief Class for controlling the VESC BLDC motors.
 *
 * Includes callbacks for reading the current state, functions to modify and
 * retrieve settings, and a main task responsible for transmitting parameters over CAN.
 * Inherits from MotorBase and DeviceThreadedBase, which is why it uses device_start(),
 * device_stop() and device_reset() methods from DeviceThreadedBase
 */

class VescMotor : public MotorBase, public DeviceThreadedBase {
public:
  /**
   * @brief Makefactory which passes parameters to the VescMotor constructor
   * @param can CAN interface used for communication between the devices both for read and write instructions. Can't be NULL.
   * @param timer Timer mostly used to track timeouts. If not passed as a parameter, will default to a new global timer.
   * @note Use it to initialize every instance of VescMotor, the constructor itself is private.
   */
  static Result<std::shared_ptr<VescMotor>> Make(std::shared_ptr<CanBase> can, std::shared_ptr<Timer> timer = nullptr);
  VescMotor &operator=(const VescMotor &) = delete;

  /// @brief Get the current speed of the VescMotor
  /// @return The speed in radians per second
  [[nodiscard]] float get_velocity() const override;

  /// @brief Get the current torque of the VescMotor
  /// @return The torque in Nm
  [[nodiscard]] float get_torque() const override;

  /// @brief Get the current position of the VescMotor
  /// @return The position in radians
  [[nodiscard]] float get_position() const override;

  /// @brief Get the absolute position of the VescMotor. In this case, equivalent to get_position()
  /// @return The position in radians
  [[nodiscard]] float get_absolute_position() const override;

  /// @brief Get the gear ratio of the VescMotor
  /// @return The gear rato - a floating point number.
  [[nodiscard]] float get_gear_ratio() const override;

  /// @brief Get all the parameters unique to VescMotor.
  /// @return An lvalue reference to the structure which holds all the parameters.
  [[nodiscard]] const VescParams &get_vesc_params() const;

  /// @brief Sets the target velocity of the VescMotor. Additionally, it switches the control mode to VELOCITY,
  /// enabling the main task to send CAN messages that regulate the motor speed toward the target value.
  /// @param speed The velocity in radians per second, can be negative or positive to change the direction.
  void set_velocity(float speed) override;

  /// @brief Sets the target torque of the VescMotor. Additionally, it switches the control mode to TORQUE,
  /// enabling the main task to send CAN messages that regulate the torque toward the target value.
  /// @param torque The torque in Nm, can be negative or positive to change the direction.
  void set_torque(float torque) override;

  /// @brief Set the target position of the VescMotor. Additionally, it switches the control mode to POSITION,
  /// enabling the main task to send CAN messages that regulate the position toward the target value.
  /// @param position The position in radians
  void set_position(float position) override;

  /// @brief Enable or disable the VescMotor - it sets the target velocity to 0 in order to
  /// stop unnecessary movement but also prevent a timeout.
  /// @param enable True to enable the Motor, false to disable it.
  void set_enable(bool enable) override;

  /// @brief Set the gear ratio of the VescMotor, which means that motor will have to rotate N times more to rotate the output once.
  void set_gear_ratio(float gear_ratio) override;

  /// @brief Set the max velocity of the VescMotor
  void set_max_velocity(float max_velocity) override;

  /// @brief Set the min velocity of the VescMotor
  void set_min_velocity(float min_velocity) override;

  /// @brief Set the reverse of the VescMotor
  void set_reverse(bool reverse) override;

  /// @brief Check if the device is operating normally.
  /// @return bool True if the device is operating normally, false otherwise.
  bool device_ok() override;

  /// @brief Check if the device is connected using timeout triggers.
  /// @return Result<bool> True if the device is connected, false otherwise (that is if the timeout timer has triggered).
  Result<bool> device_is_connected() override;

  /// @brief Get the status of the device, for example, if the device is connected, powered on,
  /// or if there is an error. Private status class member is set by other functions.
  /// @return Status Device status.
  Status device_get_status() override;

  /// @brief Set the settings for the device. DeviceSettings is cast to VescMotorSettings and validated.
  /// @param settings settings for the device passed as a reference to DeviceSettings struct.
  /// @return Status OK if settings were set.
  Status device_set_settings(const DeviceSettings &settings) override;

private:
  VescMotor(std::shared_ptr<CanBase> can, std::shared_ptr<Timer> timer);

  Status do_device_task_start() override;
  Status do_device_task_stop() override;
  Status do_device_task_reset() override;

  Status init();
  Status stop();

  static Status task_before(SimpleTask &handler, void *arg);
  static Status task(SimpleTask &handler, void *arg);
  Status handle();

  std::shared_ptr<CanBase> can;
  std::shared_ptr<Timer> timer;

  VescMotorSettings settings;

  movement::MovementControlMode control_mode;
  movement::MovementControlMode target_control_mode;

  float steps_per_revolution;
  // float gear_ratio;
  float max_velocity;
  float min_velocity;
  bool reverse;
  bool enabled;
  Status status;

  movement::MovementState current_state;
  movement::MovementState target_state;
  VescParams vesc_params;

  static void inline can_callback_status_1(CanBase &can, CanDataFrame &msg, void *args);
  static void inline can_callback_status_2(CanBase &can, CanDataFrame &msg, void *args);
  static void inline can_callback_status_3(CanBase &can, CanDataFrame &msg, void *args);
  static void inline can_callback_status_4(CanBase &can, CanDataFrame &msg, void *args);
  static void inline can_callback_status_5(CanBase &can, CanDataFrame &msg, void *args);
  static void inline can_callback_status_6(CanBase &can, CanDataFrame &msg, void *args);

  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_1_FRAME_ID       = 0x914u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_2_FRAME_ID       = 0xE14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_3_FRAME_ID       = 0xF14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_4_FRAME_ID       = 0x1014u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_5_FRAME_ID       = 0x1B14u;
  static constexpr uint32_t CAN_VESC_FLEFT_STATUS_6_FRAME_ID       = 0x1C14u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_RPM_FRAME_ID        = 0x314u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_RPM_LENGTH          = 4u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_RPM_IS_EXTENDED     = 1;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_POS_FRAME_ID        = 0x414u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_POS_LENGTH          = 4u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_POS_IS_EXTENDED     = 1;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_CURRENT_FRAME_ID    = 0x114u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_CURRENT_LENGTH      = 4u;
  static constexpr uint32_t CAN_VESC_FLEFT_SET_CURRENT_IS_EXTENDED = 1;

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
    int32_t watt_hours;
    int32_t watt_hours_chg;
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

  struct can_vesc_fleft_set_rpm_t {
    int32_t rpm;
  };

  struct can_vesc_fleft_set_pos_t {
    int32_t position;
  };

  struct can_vesc_fleft_set_current_t {
    int32_t current;
  };


  static uint16_t unpack_left_shift_u16(uint8_t value, uint8_t shift, uint8_t mask);
  static uint32_t unpack_left_shift_u32(uint8_t value, uint8_t shift, uint8_t mask);
  static uint16_t unpack_right_shift_u16(uint8_t value, uint8_t shift, uint8_t mask);
  static uint32_t unpack_right_shift_u32(uint8_t value, uint8_t shift, uint8_t mask);
  static uint8_t pack_left_shift_u32(uint32_t value, uint8_t shift, uint8_t mask);
  static uint8_t pack_right_shift_u32(uint32_t value, uint8_t shift, uint8_t mask);

  static int can_vesc_fleft_status_1_unpack(struct can_vesc_fleft_status_1_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_2_unpack(struct can_vesc_fleft_status_2_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_3_unpack(struct can_vesc_fleft_status_3_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_4_unpack(struct can_vesc_fleft_status_4_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_5_unpack(struct can_vesc_fleft_status_5_t *dst_p, const uint8_t *src_p, size_t size);
  static int can_vesc_fleft_status_6_unpack(struct can_vesc_fleft_status_6_t *dst_p, const uint8_t *src_p, size_t size);

  static int can_vesc_fleft_set_rpm_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_rpm_t *src_p, size_t size);
  static int can_vesc_fleft_set_pos_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_pos_t *src_p, size_t size);
  static int can_vesc_fleft_set_current_pack(uint8_t *dst_p, const struct can_vesc_fleft_set_current_t *src_p, size_t size);
};

} // namespace stmepic::motor

/** @} */
/** @} */