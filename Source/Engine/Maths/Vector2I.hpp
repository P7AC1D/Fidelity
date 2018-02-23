#pragma once
#include "../Core/Types.hpp"
#include "Vector2.hpp"

class Vector2I
{
public:
  static Vector2I Identity;
  static Vector2I Zero;

  Vector2I();
  Vector2I(int32 a);
  Vector2I(int32 x, int32 y);
  Vector2I(const Vector2I& rhs);

  Vector2I& operator=(const Vector2I& rhs);
  Vector2I operator+(const Vector2I& rhs);
  Vector2I operator-(const Vector2I& rhs);

  Vector2I operator+(int32 rhs);
  Vector2I operator-(int32 rhs);
  Vector2I operator*(int32 rhs);

  Vector2I& operator+=(const Vector2I& rhs);
  Vector2I& operator-=(const Vector2I& rhs);

  Vector2I& operator+=(int32 rhs);
  Vector2I& operator-=(int32 rhs);
  Vector2I& operator*=(int32 rhs);

  bool operator==(const Vector2I& rhs) const;
  bool operator!=(const Vector2I& rhs) const;

  int32& operator[](uint32 i);
  int32 operator[](uint32 i) const;

  const int32* Ptr() const;

  friend Vector2I operator+(int32 a, const Vector2I& b);
  friend Vector2I operator-(int32 a, const Vector2I& b);
  friend Vector2I operator*(int32 a, const Vector2I& b);

public:
  int32 X;
  int32 Y;
};
