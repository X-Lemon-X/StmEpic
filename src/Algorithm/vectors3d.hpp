#pragma once

#include "stmepic.hpp"


namespace stmepic::algorithm{

template <typename T>
struct Vector3d_t{
  using type = T;
  static_assert(std::is_arithmetic<type>::value, "Vector3d_t<T> requires T to be an arithmetic type");
  T x;
  T y;
  T z;

  Vector3d_t<T> operator+(const Vector3d_t<T>& rhs){
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }

  Vector3d_t<T> operator-(const Vector3d_t<T>& rhs){
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }

  Vector3d_t<T> operator*(const T& rhs){
    return {x * rhs, y * rhs, z * rhs};
  }

  Vector3d_t<T> operator/(const T& rhs){
    return {x / rhs, y / rhs, z / rhs};
  }

  Vector3d_t<T> operator+=(const Vector3d_t<T>& rhs){
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vector3d_t<T> operator-=(const Vector3d_t<T>& rhs){
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  Vector3d_t<T> operator*=(const T& rhs){
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Vector3d_t<T> operator/=(const T& rhs){
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  bool operator==(const Vector3d_t<T>& rhs){
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  bool operator!=(const Vector3d_t<T>& rhs){
    return x != rhs.x || y != rhs.y || z != rhs.z;
  }
};

template <typename T>
struct Vector4d_t{
  using type = T;
  T x;
  T y;
  T z;
  T w;

  Vector4d_t<T> operator+(const Vector4d_t<T>& rhs){
    return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
  }

  Vector4d_t<T> operator-(const Vector4d_t<T>& rhs){
    return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
  }

  Vector4d_t<T> operator*(const T& rhs){
    return {x * rhs, y * rhs, z * rhs, w * rhs};
  }

  Vector4d_t<T> operator/(const T& rhs){
    return {x / rhs, y / rhs, z / rhs, w / rhs};
  }

  Vector4d_t<T> operator+=(const Vector4d_t<T>& rhs){
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  Vector4d_t<T> operator-=(const Vector4d_t<T>& rhs){
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
  }

  Vector4d_t<T> operator*=(const T& rhs){
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
  }

  Vector4d_t<T> operator/=(const T& rhs){
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
  }

  bool operator==(const Vector4d_t<T>& rhs){
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }

  bool operator!=(const Vector4d_t<T>& rhs){
    return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
  }
};

} // namespace stmepic::algorithm