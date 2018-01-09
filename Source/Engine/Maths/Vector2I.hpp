#pragma once
#include "../Core/Types.hpp"

class Vector2i
{
public:
  static Vector2i Identity;
  static Vector2i Zero;

  Vector2i();
  Vector2i(int32 a);
  Vector2i(int32 x, int32 y);
  Vector2i(const Vector2i& rhs);

  Vector2i& operator=(const Vector2i& rhs);
  Vector2i operator+(const Vector2i& rhs);
  Vector2i operator-(const Vector2i& rhs);

  Vector2i operator+(int32 rhs);
  Vector2i operator-(int32 rhs);
  Vector2i operator*(int32 rhs);

  Vector2i& operator+=(const Vector2i& rhs);
  Vector2i& operator-=(const Vector2i& rhs);

  Vector2i& operator+=(int32 rhs);
  Vector2i& operator-=(int32 rhs);
  Vector2i& operator*=(int32 rhs);

  bool operator==(const Vector2i& rhs) const;
  bool operator!=(const Vector2i& rhs) const;

  int32& operator[](uint32 i);
  int32 operator[](uint32 i) const;

  const int32* Ptr() const;

  friend Vector2i operator+(int32 a, const Vector2i& b);
  friend Vector2i operator-(int32 a, const Vector2i& b);
  friend Vector2i operator*(int32 a, const Vector2i& b);

private:
  int32 _x;
  int32 _y;
};
