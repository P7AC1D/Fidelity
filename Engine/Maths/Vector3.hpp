#pragma once
#include "Types.h"

class Vector2;

class Vector3
{
public:
  static Vector3 Zero;
  static Vector3 One;
  
  static float32 Dot(const Vector3& a, const Vector3& b);
  static Vector3 Cross(const Vector3& a, const Vector3& b);
  
  Vector3();
  Vector3(float32 k);
  Vector3(float32 a, float32 b, float32 c);
  Vector3(const Vector2& vec, float32 k = 0.0f);
  Vector3(const Vector3& vec);
  
  Vector3& operator=(const Vector3& rhs);
  Vector3 operator+(const Vector3& rhs);
  Vector3 operator-(const Vector3& rhs);
  
  Vector3 operator+(float32 rhs);
  Vector3 operator-(float32 rhs);
  Vector3 operator*(float32 rhs);
  
  Vector3& operator+=(const Vector3& rhs);
  Vector3& operator-=(const Vector3& rhs);
  
  Vector3& operator+=(float32 rhs);
  Vector3& operator-=(float32 rhs);
  Vector3& operator*=(float32 rhs);
  
  bool operator==(const Vector3& rhs) const;
  bool operator!=(const Vector3& rhs) const;
  
  float32& operator[](uint32 i);
  float32 operator[](uint32 i) const;
  
  float32 Length() const;
  void Normalize();
  
  friend Vector3 operator+(float32 lhs, const Vector3& rhs);
  friend Vector3 operator-(float32 lhs, const Vector3& rhs);
  friend Vector3 operator*(float32 lhs, const Vector3& rhs);
  
private:
  float32 _x;
  float32 _y;
  float32 _z;
};

