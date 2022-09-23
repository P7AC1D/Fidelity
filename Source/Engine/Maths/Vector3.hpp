#pragma once
#include "../Core/Types.hpp"

class Vector2;
class Vector4;

class Vector3
{
public:
  static Vector3 Zero;
  static Vector3 Up;
  static Vector3 Identity;

  static float32 Length(const Vector3 &a);
  static float32 Dot(const Vector3 &a, const Vector3 &b);
  static Vector3 Cross(const Vector3 &a, const Vector3 &b);
  static Vector3 Normalize(const Vector3 &vec);

  Vector3();
  Vector3(float32 k);
  Vector3(float32 a, float32 b, float32 c);
  Vector3(const Vector2 &vec, float32 k = 0.0f);
  Vector3(const Vector3 &vec);
  explicit Vector3(const Vector4 &vec);

  Vector3 operator-() const;

  Vector3 &operator=(const Vector3 &rhs);
  Vector3 operator+(const Vector3 &rhs) const;
  Vector3 operator-(const Vector3 &rhs) const;
  Vector3 operator*(const Vector3 &rhs) const;
  Vector3 operator/(const Vector3 &rhs) const;

  Vector3 operator+(float32 rhs) const;
  Vector3 operator-(float32 rhs) const;
  Vector3 operator*(float32 rhs) const;
  Vector3 operator/(float32 rhs) const;

  Vector3 &operator+=(const Vector3 &rhs);
  Vector3 &operator-=(const Vector3 &rhs);

  Vector3 &operator+=(float32 rhs);
  Vector3 &operator-=(float32 rhs);
  Vector3 &operator*=(float32 rhs);
  Vector3 &operator/=(float32 rhs);
  Vector3 &operator*=(const Vector3 &rhs);

  bool operator==(const Vector3 &rhs) const;
  bool operator!=(const Vector3 &rhs) const;

  float32 &operator[](uint32 i);
  const float32 &operator[](uint32 i) const;

  float32 Length() const;
  void Normalize();

  const float32 *Ptr() const;

  friend Vector3 operator+(float32 lhs, const Vector3 &rhs);
  friend Vector3 operator-(float32 lhs, const Vector3 &rhs);
  friend Vector3 operator*(float32 lhs, const Vector3 &rhs);

public:
  float32 X;
  float32 Y;
  float32 Z;
};
