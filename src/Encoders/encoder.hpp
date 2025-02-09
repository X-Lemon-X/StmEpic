#pragma once
#include "Timing.hpp"
#include "device.hpp"
#include "filter.hpp"
#include "stmepic.hpp"

#define VELOCITY_FILTER_SIZE 6
#define ANGLE_MAX_DEFFERENCE 2.0f // 1 radian

/**
 * @file encoder.hpp
 * @brief Base classes for all encoders.
 */

/**
 * @defgroup Encoders
 * @brief Encoders module for base encoder interface with encoder implementations.
 * For reading data from absolute and incremental encoders.
 * @{
 */


namespace stmepic::encoders {

/**
 * @class EncoderBase
 * @brief Base Interface for all encoders.
 *
 */
class EncoderBase : public DeviceThreadedBase {
public:
  /// @brief Construct a new Encoder Base object
  EncoderBase()          = default;
  virtual ~EncoderBase() = default;

  /// @brief reads the last calculated velocity
  /// @return the velocity in radians per second
  [[nodiscard]] virtual float get_velocity() const = 0;

  /// @brief reads the last calculated torque,
  /// technically the torque is usually not calculated by the encoder but whatever.
  /// it't easier to write class tha will inherit from some encoder and add torque reading
  /// functionality then add another class for torque reading.
  /// @return the torque in Nm
  [[nodiscard]] virtual float get_torque() const = 0;

  /// @brief gets the last calcualated angle with offset and all other corrections.
  /// Only single rotation in two directions form -2pi to  0 to 2pi.
  /// @return the angle in radians
  [[nodiscard]] virtual float get_angle() const = 0;

  /// @brief gets the latest calculated absolute angle
  /// absoulte angle includes information how many times the encoder has rotated
  /// for example if the encoder has rotated 3 times the angle will be 6pi + the current angle
  /// @return the absoulte angle in radians
  [[nodiscard]] virtual float get_absoulute_angle() const = 0;

  /// @brief sets the offset of the encoder
  /// @param offset the offset in radians
  virtual void set_offset(float offset) = 0;

  /// @brief sets the reverse of the encoder
  /// @param reverse true if the encoder is reversed
  virtual void set_reverse(bool reverse) = 0;

  /// @brief set the ratio that will be multiplayed by value of the angle and velocity thus
  /// is used to calculate the real angle and velocity of the motor in case the motor has a gear reduction
  /// @param ratio
  virtual void set_ratio(float ratio) = 0;

protected:
  /// @brief Shoule be called to initialize the encoder afer changign the settings
  virtual void init() = 0;
};

} // namespace stmepic::encoders
