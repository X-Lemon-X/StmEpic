#pragma once

#include "stmepic.hpp"


namespace stmepic::algorithm {

template <typename T> struct Vector3d_t {
  using type = T;
  static_assert(std::is_arithmetic<type>::value, "Vector3d_t<type> requires type to be an arithmetic type");
  type x;
  type y;
  type z;

  Vector3d_t<type> operator+(const Vector3d_t<type> &rhs) {
    return { x + rhs.x, y + rhs.y, z + rhs.z };
  }

  Vector3d_t<type> operator-(const Vector3d_t<type> &rhs) {
    return { x - rhs.x, y - rhs.y, z - rhs.z };
  }

  Vector3d_t<type> operator*(const type &rhs) {
    return { x * rhs, y * rhs, z * rhs };
  }

  Vector3d_t<type> operator/(const type &rhs) {
    return { x / rhs, y / rhs, z / rhs };
  }

  Vector3d_t<type> operator+=(const Vector3d_t<type> &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vector3d_t<type> operator-=(const Vector3d_t<type> &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  Vector3d_t<type> operator*=(const type &rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Vector3d_t<type> operator/=(const type &rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  bool operator==(const Vector3d_t<type> &rhs) {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  bool operator!=(const Vector3d_t<type> &rhs) {
    return x != rhs.x || y != rhs.y || z != rhs.z;
  }
};

template <typename T> struct Vector4d_t {
  using type = T;
  type x;
  type y;
  type z;
  type w;

  Vector4d_t<type> operator+(const Vector4d_t<type> &rhs) {
    return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
  }

  Vector4d_t<type> operator-(const Vector4d_t<type> &rhs) {
    return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
  }

  Vector4d_t<type> operator*(const type &rhs) {
    return { x * rhs, y * rhs, z * rhs, w * rhs };
  }

  Vector4d_t<type> operator/(const type &rhs) {
    return { x / rhs, y / rhs, z / rhs, w / rhs };
  }

  Vector4d_t<type> operator+=(const Vector4d_t<type> &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  Vector4d_t<type> operator-=(const Vector4d_t<type> &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
  }

  Vector4d_t<type> operator*=(const type &rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
  }

  Vector4d_t<type> operator/=(const type &rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
  }

  bool operator==(const Vector4d_t<type> &rhs) {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }

  bool operator!=(const Vector4d_t<type> &rhs) {
    return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
  }
};

} // namespace stmepic::algorithm