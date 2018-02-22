#pragma once
#include "../Core/Types.hpp"

class Degree;
class Matrix3;
class Radian;
class Vector3;

class Quaternion
{
public:
  static Quaternion Zero;
  static Quaternion Identity;
  
  static float32 Dot(const Quaternion& lhs, const Quaternion& rhs);
  static Quaternion Normalize(const Quaternion& quat);
  
  Quaternion();
  Quaternion(float32 w, float32 x, float32 y, float32 z);
  explicit Quaternion(const Matrix3& rotMat);
  explicit Quaternion(const Vector3& axis, const Radian& angle);
  explicit Quaternion(const Degree& xAngle, const Degree& yAngle, const Degree& zAngle);
  explicit Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle);
  
  float32& operator[](uint32 i);
  float32 operator[](uint32 i) const;
  
  Quaternion& operator=(const Quaternion& rhs);
  
  Quaternion operator+(float32 rhs) const;
  Quaternion operator-(float32 rhs) const;
  Quaternion operator*(float32 rhs) const;
  Quaternion operator+(const Quaternion& rhs) const;
  Quaternion operator-(const Quaternion& rhs) const;
  Quaternion operator*(const Quaternion& rhs) const;
  
  Quaternion& operator+=(float32 rhs);
  Quaternion& operator-=(float32 rhs);
  Quaternion& operator*=(float32 rhs);
  Quaternion& operator+=(const Quaternion& rhs);
  Quaternion& operator-=(const Quaternion& rhs);
  Quaternion& operator*=(const Quaternion& rhs);
  
  bool operator==(const Quaternion& rhs) const;
  bool operator!=(const Quaternion& rhs) const;
  
  float32 Norm() const;
  void Normalize();

  Vector3 Rotate(const Vector3& vec) const;
  
  // TBD: Unit test these guys
  Quaternion Lerp(const Quaternion& a, const Quaternion& b, float32 t);
  Quaternion Slerp(const Quaternion& a, const Quaternion& b, float32 t);
  
  friend Quaternion operator+(float32 lhs, const Quaternion& rhs);
  friend Quaternion operator-(float32 lhs, const Quaternion& rhs);
  friend Quaternion operator*(float32 lhs, const Quaternion& rhs);
  
private:
  static Quaternion Multiply(const Quaternion& q1, const Quaternion& q2);
  void FromEulerAngles(const Radian& pitch, const Radian& yaw, const Radian& roll);
  void FromAxisAngle(const Vector3& axis, const Radian& angle);
  
public:
  float32 X;
  float32 Y;
  float32 Z;
  float32 W;
};
