#pragma once
#include "../Core/Types.hpp"

class Quaternion;
class Matrix4;
class Vector3;

class Matrix3
{
public:
  static Matrix3 Zero;
  static Matrix3 Identity;
  static Matrix3 LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);

  Matrix3();
  Matrix3(const Matrix3 &mat);
  Matrix3(const Matrix4 &mat);
  Matrix3(float32 a, float32 b, float32 c,
          float32 d, float32 e, float32 f,
          float32 g, float32 h, float32 i);
  explicit Matrix3(const Quaternion &qat);

  Matrix3 operator+(float32 rhs) const;
  Matrix3 operator-(float32 rhs) const;
  Matrix3 operator*(float32 rhs) const;
  Matrix3 operator+(const Matrix3 &rhs) const;
  Matrix3 operator-(const Matrix3 &rhs) const;
  Matrix3 operator*(const Matrix3 &rhs) const;
  Vector3 operator*(const Vector3 &rhs) const;

  Matrix3 &operator+=(float32 rhs);
  Matrix3 &operator-=(float32 rhs);
  Matrix3 &operator*=(float32 rhs);

  bool operator==(const Matrix3 &rhs) const;
  bool operator!=(const Matrix3 &rhs) const;

  Vector3 &operator[](uint32 row);
  const Vector3 &operator[](uint32 row) const;

  float32 Determinate() const;
  Matrix3 Inverse() const;
  Matrix3 Transpose() const;

  friend Matrix3 operator+(float32 lhs, const Matrix3 &rhs);
  friend Matrix3 operator-(float32 lhs, const Matrix3 &rhs);
  friend Matrix3 operator*(float32 lhs, const Matrix3 &rhs);

private:
  float32 _m[3][3];
};
