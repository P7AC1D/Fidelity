#include "Vector2I.hpp"

#include <cassert>
#include <cmath>

Vector2i Vector2i::Identity = Vector2i(1);
Vector2i Vector2i::Zero = Vector2i(0);

Vector2i::Vector2i()
{
  _x = 0;
  _y = 0;
}

Vector2i::Vector2i(int32 a)
{
  _x = a;
  _y = a;
}

Vector2i::Vector2i(int32 x, int32 y)
{
  _x = x;
  _y = y;
}

Vector2i& Vector2i::operator=(const Vector2i& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
  return *this;
}

Vector2i::Vector2i(const Vector2i& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
}

Vector2i Vector2i::operator+(const Vector2i& rhs)
{
  return Vector2i(_x + rhs._x, _y + rhs._y);
}

Vector2i Vector2i::operator-(const Vector2i& rhs)
{
  return Vector2i(_x - rhs._x, _y - rhs._y);
}

Vector2i Vector2i::operator+(int32 rhs)
{
  return Vector2i(_x + rhs, _y + rhs);
}

Vector2i Vector2i::operator-(int32 rhs)
{
  return Vector2i(_x - rhs, _y - rhs);
}

Vector2i Vector2i::operator*(int32 rhs)
{
  return Vector2i(rhs * _x, rhs * _y);
}

Vector2i& Vector2i::operator+=(const Vector2i& rhs)
{
  _x += rhs._x;
  _y += rhs._y;
  return *this;
}

Vector2i& Vector2i::operator-=(const Vector2i& rhs)
{
  _x -= rhs._x;
  _y -= rhs._y;
  return *this;
}

Vector2i& Vector2i::operator-=(int32 rhs)
{
  _x -= rhs;
  _y -= rhs;
  return *this;
}

Vector2i& Vector2i::operator+=(int32 rhs)
{
  _x += rhs;
  _y += rhs;
  return *this;
}

Vector2i& Vector2i::operator*=(int32 rhs)
{
  _x *= rhs;
  _y *= rhs;
  return *this;
}

bool Vector2i::operator==(const Vector2i& rhs) const
{
  return _x == rhs._x && _y == rhs._y;
}

bool Vector2i::operator!=(const Vector2i& rhs) const
{
  return _x != rhs._x && _y != rhs._y;
}

int32& Vector2i::operator[](uint32 i)
{
  assert(i < 2);
  return *(&_x + i);
}

int32 Vector2i::operator[](uint32 i) const
{
  assert(i < 2);
  return *(&_x + i);
}

const int32* Vector2i::Ptr() const
{
  return &_x;
}

Vector2i operator+(int32 a, const Vector2i& b)
{
  return Vector2i(a + b._x, a + b._y);
}

Vector2i operator-(int32 a, const Vector2i& b)
{
  return Vector2i(a - b._x, a - b._y);
}

Vector2i operator*(int32 a, const Vector2i& b)
{
  return Vector2i(a * b._x, a * b._y);
}
