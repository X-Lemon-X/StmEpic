#ifndef ENCODERS_H
#define ENCODERS_H

#include "stmepic.hpp"
#include "Timing.hpp"
#include "filter.hpp"

#define VELOCITY_FILTER_SIZE 6
#define ANGLE_MAX_DEFFERENCE 2.0f // 1 radian



namespace stmepic::encoders {

class EncoderBase {
public:
  
  /// @brief Init fucnion of the encoder
  EncoderBase() = default;

  /// @brief Check if the encoder is connected. if encoder don't support the connection check it should will return true
  /// @return true if the encoder is connected.
 [[nodiscard]] virtual bool is_connected() const =0;

  /// @brief reads the last calculated velocity
  /// @return the velocity in radians per second 
  [[nodiscard]] virtual float get_velocity() const =0;

  /// @brief gets the last calcualted angle with offset and all other corrections. 
  /// Only single rotation in two directions form -2pi to  0 to 2pi.
  /// @return the angle in radians
  [[nodiscard]] virtual float get_angle() const = 0;

  /// @brief gets the latest calculated absolute angle 
  /// absoulte angle includes information how many times the encoder has rotated
  /// for example if the encoder has rotated 3 times the angle will be 6pi + the current angle
  /// @return the absoulte angle in radians
  [[nodiscard]] virtual float get_absoulute_angle() const = 0;

  /// @brief handles the encoder updates data read from the encoder
  virtual void handle() = 0;

  /// @brief handles the encoder updates in some IRK.
  virtual void handle_irk() = 0;

  /// @brief sets the offset of the encoder
  /// @param offset the offset in radians
  virtual void set_offset(float offset) = 0;
  
  /// @brief sets the reverse of the encoder
  /// @param reverse true if the encoder is reversed
  virtual void set_reverse(bool reverse) = 0;

  /// @brief set the ratio that will be multiplayed by value of the angle and velocity thus
  /// this is used to calculate the real angle and velocity of the motor in case the motor has a gear reduction
  /// @param ratio 
  virtual void set_ratio(float ratio) = 0;

  /// @brief Enable the encoder
  /// @param enable true to enable the encoder
  virtual void set_enable_encoder(bool enable) = 0;
};



}


#endif // ENCODERS_H