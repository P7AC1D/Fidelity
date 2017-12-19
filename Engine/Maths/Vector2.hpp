#pragma once
#include "Types.h"

class Vector2
{
public:
  static Vector2 One;
  
  static float32 Dot(const Vector2& a, const Vector2& b);
  
  Vector2();
  Vector2(float32 a);
  Vector2(float32 x, float32 y);
  Vector2(const Vector2& rhs);
  
  Vector2& operator=(const Vector2& rhs);
  Vector2 operator+(const Vector2& rhs);
  Vector2 operator-(const Vector2& rhs);
  
  Vector2 operator+(float32 rhs);
  Vector2 operator-(float32 rhs);
  Vector2 operator*(float32 rhs);
  
  Vector2& operator+=(const Vector2& rhs);
  Vector2& operator-=(const Vector2& rhs);
  
  Vector2& operator+=(float32 rhs);
  Vector2& operator-=(float32 rhs);
  Vector2& operator*=(float32 rhs);
  
  bool operator==(const Vector2& rhs) const;
  bool operator!=(const Vector2& rhs) const;
  
  float32& operator[](uint32 i);
  float32 operator[](uint32 i) const;
  
  float32 Length() const;
  void Normalize();
  
  friend Vector2 operator+(float32 a, const Vector2& b);
  friend Vector2 operator-(float32 a, const Vector2& b);
  friend Vector2 operator*(float32 a, const Vector2& b);
  
private:
  float32 _x;
  float32 _y;
};
