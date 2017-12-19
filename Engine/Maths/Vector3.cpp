#include "Vector3.hpp"

#include <cassert>
#include <cmath>

#include "Vector2.hpp"

Vector3 Vector3::One = Vector3(1.0f);

float32 Vector3::Dot(const Vector3& a, const Vector3& b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vector3 Vector3::Cross(const Vector3 &a, const Vector3 &b)
{
  return Vector3(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

Vector3::Vector3()
{
  _x = 0.0f;
  _y = 0.0f;
  _z = 0.0f;
}

Vector3::Vector3(float32 k)
{
  _x = k;
  _y = k;
  _z = k;
}

Vector3::Vector3(float32 a, float32 b, float32 c)
{
  _x = a;
  _y = b;
  _z = c;
}

Vector3::Vector3(const Vector2& vec, float32 k)
{
  _x = vec[0];
  _y = vec[1];
  _z = k;
}

Vector3::Vector3(const Vector3& vec)
{
  _x = vec._x;
  _y = vec._y;
  _z = vec._z;
}

Vector3& Vector3::operator=(const Vector3& rhs)
{
  _x = rhs._x;
  _y = rhs._y;
  _z = rhs._z;
  return *this;
}

Vector3 Vector3::operator+(const Vector3& rhs)
{
  float32 x = _x + rhs._x;
  float32 y = _y + rhs._y;
  float32 z = _z + rhs._z;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator-(const Vector3& rhs)
{
  float32 x = _x - rhs._x;
  float32 y = _y - rhs._y;
  float32 z = _z - rhs._z;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator+(float32 rhs)
{
  float32 x = _x + rhs;
  float32 y = _y + rhs;
  float32 z = _z + rhs;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator-(float32 rhs)
{
  float32 x = _x - rhs;
  float32 y = _y - rhs;
  float32 z = _z - rhs;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator*(float32 rhs)
{
  float32 x = _x * rhs;
  float32 y = _y * rhs;
  float32 z = _z * rhs;
  return Vector3(x, y, z);
}

Vector3& Vector3::operator+=(const Vector3& rhs)
{
  _x += rhs._x;
  _y += rhs._y;
  _z += rhs._z;
  return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
  _x -= rhs._x;
  _y -= rhs._y;
  _z -= rhs._z;
  return *this;
}

Vector3& Vector3::operator+=(float32 rhs)
{
  _x += rhs;
  _y += rhs;
  _z += rhs;
  return *this;
}

Vector3& Vector3::operator-=(float32 rhs)
{
  _x -= rhs;
  _y -= rhs;
  _z -= rhs;
  return *this;
}

Vector3& Vector3::operator*=(float32 rhs)
{
  _x *= rhs;
  _y *= rhs;
  _z *= rhs;
  return *this;
}

bool Vector3::operator==(const Vector3& rhs) const
{
  return _x == rhs._x && _y == rhs._y && _z == rhs._z;
}

bool Vector3::operator!=(const Vector3& rhs) const
{
  return _x != rhs._x && _y != rhs._y && _z != rhs._z;
}

float32& Vector3::operator[](uint32 i)
{
  assert(i < 3);
  return *(&_x + i);
}

float32 Vector3::operator[](uint32 i) const
{
  assert(i < 3);
  return *(&_x + i);
}

float32 Vector3::Length() const
{
  return sqrtf(_x * _x + _y * _y + _z * _z);
}

void Vector3::Normalize()
{
  float32 length = this->Length();
  float32 lengthInv = 1.0f / length;
  _x *= lengthInv;
  _y *= lengthInv;
  _z *= lengthInv;
}

Vector3 operator+(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs + rhs._x;
  float32 y = lhs + rhs._y;
  float32 z = lhs + rhs._z;
  return Vector3(x, y, z);
}

Vector3 operator-(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs - rhs._x;
  float32 y = lhs - rhs._y;
  float32 z = lhs - rhs._z;
  return Vector3(x, y, z);
}

Vector3 operator*(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs * rhs._x;
  float32 y = lhs * rhs._y;
  float32 z = lhs * rhs._z;
  return Vector3(x, y, z);
}
