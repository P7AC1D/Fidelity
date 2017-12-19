#include "Vector2.hpp"

#include <cassert>
#include <cmath>

Vector2 Vector2::One = Vector2(1.0f);

float32 Vector2::Dot(const Vector2& a, const Vector2& b)
{
  return a[0] * b[0] + a[1] * b[1];
}

Vector2::Vector2()
{
  _x = 0.0f;
  _y = 0.0f;
}

Vector2::Vector2(float32 a)
{
  _x = a;
  _y = a;
}

Vector2::Vector2(float32 x, float32 y)
{
  _x = x;
  _y = y;
}

Vector2& Vector2::operator=(const Vector2& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
  return *this;
}

Vector2::Vector2(const Vector2& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
}

Vector2 Vector2::operator+(const Vector2& rhs)
{
  return Vector2(_x + rhs._x, _y + rhs._y);
}

Vector2 Vector2::operator-(const Vector2& rhs)
{
  return Vector2(_x - rhs._x, _y - rhs._y);
}

Vector2 Vector2::operator+(float32 rhs)
{
  return Vector2(_x + rhs, _y + rhs);
}

Vector2 Vector2::operator-(float32 rhs)
{
  return Vector2(_x - rhs, _y - rhs);
}

Vector2 Vector2::operator*(float32 rhs)
{
  return Vector2(rhs * _x, rhs * _y);
}

Vector2& Vector2::operator+=(const Vector2& rhs)
{
  _x += rhs._x;
  _y += rhs._y;
  return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
  _x -= rhs._x;
  _y -= rhs._y;
  return *this;
}

Vector2& Vector2::operator-=(float32 rhs)
{
  _x -= rhs;
  _y -= rhs;
  return *this;
}

Vector2& Vector2::operator+=(float32 rhs)
{
  _x += rhs;
  _y += rhs;
  return *this;
}

Vector2& Vector2::operator*=(float32 rhs)
{
  _x *= rhs;
  _y *= rhs;
  return *this;
}

bool Vector2::operator==(const Vector2& rhs) const
{
  return _x == rhs._x && _y == rhs._y;
}

bool Vector2::operator!=(const Vector2& rhs) const
{
  return _x != rhs._x && _y != rhs._y;
}

float32& Vector2::operator[](uint32 i)
{
  assert(i < 2);
  return *(&_x + i);
}

float32 Vector2::operator[](uint32 i) const
{
  assert(i < 2);
  return *(&_x + i);
}

float32 Vector2::Length() const
{
  return sqrtf(_x * _x + _y * _y);
}

void Vector2::Normalize()
{
  float32 length = Length();
  float32 lengthInv = 1.0f / length;
  _x *= lengthInv;
  _y *= lengthInv;
}

Vector2 operator+(float32 a, const Vector2& b)
{
  return Vector2(a + b._x, a + b._y);
}

Vector2 operator-(float32 a, const Vector2& b)
{
  return Vector2(a - b._x, a - b._y);
}

Vector2 operator*(float32 a, const Vector2& b)
{
  return Vector2(a * b._x, a * b._y);
}
