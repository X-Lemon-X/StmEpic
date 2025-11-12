#pragma once


#include "stmepic.hpp"
#include "vectors3d.hpp"
#include <string>

using namespace stmepic::algorithm;
namespace stmepic::sensors::imu {

struct ImuData {
  int8_t temp;
  Vector3d_t<float> acceleration; // acceleration in XYZ in m/s^2
  Vector3d_t<float> gyration;
  Vector3d_t<float> magnetic_field;

  Vector3d_t<float> linear_acceleration;
  Vector3d_t<float> gravity;
  Vector3d_t<float> euler_angles;
  Vector4d_t<float> quaternion;

  std::string to_string() const {
    using std::to_string;
    auto i = [](auto v) { return to_string(static_cast<int>(v)); };
    return "Temp: " + to_string(static_cast<int>(temp)) + " C, Acc: [" + i(acceleration.x) + ", " +
           i(acceleration.y) + ", " + i(acceleration.z) + "] m/s^2, Gyration: [" + i(gyration.x) + ", " +
           i(gyration.y) + ", " + i(gyration.z) + "] dps, Magnetic Field: [" + i(magnetic_field.x) + ", " +
           i(magnetic_field.y) + ", " + i(magnetic_field.z) + "] uT, Linear Acc: [" + i(linear_acceleration.x) +
           ", " + i(linear_acceleration.y) + ", " + i(linear_acceleration.z) + "] m/s^2, Gravity: [" +
           i(gravity.x) + ", " + i(gravity.y) + ", " + i(gravity.z) + "] m/s^2, Euler Angles: [" +
           i(euler_angles.x) + ", " + i(euler_angles.y) + ", " + i(euler_angles.z) + "] rad, Quaternion: [" +
           i(quaternion.w) + ", " + i(quaternion.x) + ", " + i(quaternion.y) + ", " + i(quaternion.z) + "]";
  }
};


class IMU : public virtual stmepic::DeviceBase {
public:
  virtual ~IMU()                     = default;
  virtual Result<ImuData> get_data() = 0;
};

} // namespace stmepic::sensors::imu