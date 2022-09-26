#pragma once
#include "../Core/Types.hpp"
#include "Matrix3.hpp"
#include "Plane.hpp"
#include "Quaternion.hpp"
#include "Radian.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

class Matrix4
{
public:
  static Matrix4 Zero;
  static Matrix4 Identity;
  static Matrix4 Translation(const Vector3 &translation);
  static Matrix4 Scaling(const Vector3 &scale);
  static Matrix4 Rotation(const Quaternion &rotation);
  static Matrix4 LookAt(const Vector3 &eye, const Vector3 &center, const Vector3 &up);
  static Matrix4 Perspective(const Radian &yFov, float32 aspect, float32 zNear, float32 zFar);
  static Matrix4 Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 zNear, float32 zFar);
  static Matrix3 ToMatrix3(const Matrix4 &mat);

  Matrix4();
  Matrix4(const Matrix4 &mat);
  Matrix4(const Matrix3 &mat);
  Matrix4(float32 a, float32 b, float32 c, float32 d,
          float32 e, float32 f, float32 g, float32 h,
          float32 i, float32 j, float32 k, float32 l,
          float32 m, float32 n, float32 o, float32 p);
  explicit Matrix4(const Quaternion &qat);

  Matrix4 operator+(float32 rhs) const;
  Matrix4 operator-(float32 rhs) const;
  Matrix4 operator*(float32 rhs) const;
  Matrix4 operator+(const Matrix4 &rhs) const;
  Matrix4 operator-(const Matrix4 &rhs) const;
  Matrix4 operator*(const Matrix4 &rhs) const;
  Vector3 operator*(const Vector3 &rhs) const;
  Vector4 operator*(const Vector4 &rhs) const;

  bool operator==(const Matrix4 &rhs) const;
  bool operator!=(const Matrix4 &rhs) const;

  Vector4 &operator[](uint32 row);
  const Vector4 &operator[](uint32 row) const;

  float32 Determinate() const;
  Matrix4 Inverse() const;
  Matrix4 Transpose() const;

  friend Matrix4 operator+(float32 lhs, const Matrix4 &rhs);
  friend Matrix4 operator-(float32 lhs, const Matrix4 &rhs);
  friend Matrix4 operator*(float32 lhs, const Matrix4 &rhs);

private:
  Vector4 _m[4];
};
