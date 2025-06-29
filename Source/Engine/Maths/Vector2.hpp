#pragma once
#include "../Core/Types.hpp"
#include "Vector2I.hpp"

class Vector2
{
public:
  static Vector2 Identity;
  
  static float32 Dot(const Vector2& a, const Vector2& b);
  static Vector2 Normalize(const Vector2& vec);
  
  Vector2();
  Vector2(float32 a);
  Vector2(float32 x, float32 y);
  Vector2(const Vector2& rhs);
  
  Vector2& operator=(const Vector2& rhs);
  Vector2 operator+(const Vector2& rhs) const;
  Vector2 operator-(const Vector2& rhs) const;
  
  Vector2 operator+(float32 rhs) const;
  Vector2 operator-(float32 rhs) const;
  Vector2 operator*(float32 rhs) const;
  
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

  const float32* Ptr() const;
  
  friend Vector2 operator+(float32 a, const Vector2& b);
  friend Vector2 operator-(float32 a, const Vector2& b);
  friend Vector2 operator*(float32 a, const Vector2& b);

public:
  float32 X;
  float32 Y;
};
