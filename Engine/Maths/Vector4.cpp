#include "Vector4.hpp"

#include <cassert>
#include <cmath>

#include "Vector2.hpp"
#include "Vector3.hpp"

Vector4 Vector4::One = Vector4(1.0f);

float32 Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
{
  return lhs._x * rhs._x + lhs._y * rhs._y + lhs._z * rhs._z + lhs._w * rhs._w;
}

Vector4::Vector4()
{
  _x = 0.0f;
  _y = 0.0f;
  _z = 0.0f;
  _w = 0.0f;
}

Vector4::Vector4(float32 k)
{
  _x = k;
  _y = k;
  _z = k;
  _w = k;
}

Vector4::Vector4(float32 a, float32 b, float32 c, float32 d)
{
  _x = a;
  _y = b;
  _z = c;
  _w = d;
}

Vector4::Vector4(const Vector2& vec, float32 c, float32 d)
{
  _x = vec[0];
  _y = vec[1];
  _z = c;
  _w = d;
}

Vector4::Vector4(const Vector3& vec, float32 d)
{
  _x = vec[0];
  _y = vec[1];
  _z = vec[2];
  _w = d;
}

Vector4::Vector4(const Vector4& vec)
{
  _x = vec[0];
  _y = vec[1];
  _z = vec[2];
  _w = vec[3];
}

Vector4& Vector4::operator=(const Vector4& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
  _z = rhs._z;
  _w = rhs._w;
  return *this;
}

Vector4 Vector4::operator+(const Vector4 &rhs)
{
  float32 x = _x + rhs._x;
  float32 y = _y + rhs._y;
  float32 z = _z + rhs._z;
  float32 w = _w + rhs._w;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator-(const Vector4 &rhs)
{
  float32 x = _x - rhs._x;
  float32 y = _y - rhs._y;
  float32 z = _z - rhs._z;
  float32 w = _w - rhs._w;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator+(float32 rhs)
{
  float32 x = _x + rhs;
  float32 y = _y + rhs;
  float32 z = _z + rhs;
  float32 w = _w + rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator-(float32 rhs)
{
  float32 x = _x - rhs;
  float32 y = _y - rhs;
  float32 z = _z - rhs;
  float32 w = _w - rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator*(float32 rhs)
{
  float32 x = _x * rhs;
  float32 y = _y * rhs;
  float32 z = _z * rhs;
  float32 w = _w * rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator+=(const Vector4& rhs)
{
  _x += rhs._x;
  _y += rhs._y;
  _z += rhs._z;
  _w += rhs._w;
  return *this;
}

Vector4 Vector4::operator-=(const Vector4& rhs)
{
  _x -= rhs._x;
  _y -= rhs._y;
  _z -= rhs._z;
  _w -= rhs._w;
  return *this;
}

Vector4& Vector4::operator+=(float32 rhs)
{
  _x += rhs;
  _y += rhs;
  _z += rhs;
  _w += rhs;
  return *this;
}

Vector4& Vector4::operator-=(float32 rhs)
{
  _x -= rhs;
  _y -= rhs;
  _z -= rhs;
  _w -= rhs;
  return *this;
}

Vector4& Vector4::operator*=(float32 rhs)
{
  _x *= rhs;
  _y *= rhs;
  _z *= rhs;
  _w *= rhs;
  return *this;
}

bool Vector4::operator==(const Vector4& rhs) const
{
  return _x == rhs._x && _y == rhs._y && _z && rhs._z && _w == rhs._w;
}

bool Vector4::operator!=(const Vector4& rhs) const
{
  return _x != rhs._x && _y != rhs._y && _z != rhs._z && _w != rhs._w;
}

float32& Vector4::operator[](uint32 i)
{
  assert(i < 4);
  return *(&_x + i);
}

float32 Vector4::operator[](uint32 i) const
{
  assert(i < 4);
  return *(&_x + i);
}

float32 Vector4::Length() const
{
  return sqrtf(_x * _x + _y * _y + _z * _z + _w * _w);
}

void Vector4::Normalize()
{
  float32 length = Length();
  float32 lengthInv = 1.0f / length;
  _x *= lengthInv;
  _y *= lengthInv;
  _z *= lengthInv;
  _w *= lengthInv;
}

Vector4 operator+(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs + rhs._x;
  float32 y = lhs + rhs._y;
  float32 z = lhs + rhs._z;
  float32 w = lhs + rhs._w;
  return Vector4(x, y, z, w);
}

Vector4 operator-(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs - rhs._x;
  float32 y = lhs - rhs._y;
  float32 z = lhs - rhs._z;
  float32 w = lhs - rhs._w;
  return Vector4(x, y, z, w);
}

Vector4 operator*(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs * rhs._x;
  float32 y = lhs * rhs._y;
  float32 z = lhs * rhs._z;
  float32 w = lhs * rhs._w;
  return Vector4(x, y, z, w);
}
