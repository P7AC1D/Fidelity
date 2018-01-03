#pragma once
#include "Types.h"

class Matrix3;
class Quaternion;
class Vector3;
class Vector4;

class Matrix4
{
public:
  static Matrix4 Zero;
  static Matrix4 Identity;
  static Matrix4 Transform(const Vector3& position, const Vector3& scale, const Quaternion& orientation);
  
  Matrix4();
  Matrix4(const Matrix4& mat);
  Matrix4(const Matrix3& mat);
  Matrix4(float32 a, float32 b, float32 c, float32 d,
          float32 e, float32 f, float32 g, float32 h,
          float32 i, float32 j, float32 k, float32 l,
          float32 m, float32 n, float32 o, float32 p);
  explicit Matrix4(const Quaternion& qat);
  
  Matrix4 operator+(float32 rhs) const;
  Matrix4 operator-(float32 rhs) const;
  Matrix4 operator*(float32 rhs) const;
  Matrix4 operator+(const Matrix4& rhs) const;
  Matrix4 operator-(const Matrix4& rhs) const;
  Matrix4 operator*(const Matrix4& rhs) const;
  Vector4 operator*(const Vector4& rhs) const;
  
  bool operator==(const Matrix4& rhs) const;
  bool operator!=(const Matrix4& rhs) const;
  
  Vector4& operator[](uint32 row);
  const Vector4& operator[](uint32 row) const;
  
  float32 Determinate() const;
  Matrix4 Inverse() const;
  Matrix4 Transpose() const;
  
  friend Matrix4 operator+(float32 lhs, const Matrix4& rhs);
  friend Matrix4 operator-(float32 lhs, const Matrix4& rhs);
  friend Matrix4 operator*(float32 lhs, const Matrix4& rhs);
  friend Vector4 operator*(const Vector4& lhs, const Matrix4& rhs);
  
private:
  float32 _m[4][4];
};
