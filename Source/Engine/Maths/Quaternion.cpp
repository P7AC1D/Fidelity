#include "Quaternion.hpp"

#include <cassert>

#include "Degree.hpp"
#include "Math.hpp"
#include "Matrix3.hpp"
#include "Radian.hpp"
#include "Vector3.hpp"

Quaternion Quaternion::Zero = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
Quaternion Quaternion::Identity = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);

float32 Quaternion::Dot(const Quaternion& lhs, const Quaternion& rhs)
{
  return lhs._x * rhs._x + lhs._y * rhs._y + lhs._z * rhs._z + lhs._w * rhs._w;
}

Quaternion Quaternion::Normalize(const Quaternion& quat)
{
  float32 norm = quat.Norm();
  float32 normInv = 1.0f / norm;
  return Quaternion(quat[0] * normInv, quat[1] * normInv, quat[2] * normInv, quat[3] * normInv);
}

Quaternion::Quaternion():
_x(0.0f),
_y(0.0f),
_z(0.0f),
_w(0.0f)
{
}

Quaternion::Quaternion(float32 _w, float32 _x, float32 _y, float32 _z):
_x(_x),
_y(_y),
_z(_z),
_w(_w)
{
}

Quaternion::Quaternion(const Matrix3& rotMax)
{
}

Quaternion::Quaternion(const Vector3& axis, const Radian& angle)
{
  FromAxisAngle(axis, angle);
}

Quaternion::Quaternion(const Degree& xAngle, const Degree& yAngle, const Degree& zAngle)
{
  FromEulerAngles(xAngle.InRadians(), yAngle.InRadians(), zAngle.InRadians());
}

Quaternion::Quaternion(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
{
  FromEulerAngles(xAngle, yAngle, zAngle);
}

float32& Quaternion::operator[](uint32 i)
{
  assert(i < 4);
  return *(&_x + i);
}

float32 Quaternion::operator[](uint32 i) const
{
  assert(i < 4);
  return *(&_x + i);
}

Quaternion& Quaternion::operator=(const Quaternion& rhs)
{
  _w = rhs._w;
  _z = rhs._z;
  _y = rhs._y;
  _x = rhs._x;
  return *this;
}

Quaternion Quaternion::operator+(float32 rhs) const
{
  return Quaternion(_w + rhs, _x + rhs, _y + rhs, _z + rhs);
}

Quaternion Quaternion::operator-(float32 rhs) const
{
  return Quaternion(_w - rhs, _x - rhs, _y - rhs, _z - rhs);
}

Quaternion Quaternion::operator*(float32 rhs) const
{
  return Quaternion(_w * rhs, _x * rhs, _y * rhs, _z * rhs);
}

Quaternion Quaternion::operator+(const Quaternion &rhs) const
{
  return Quaternion(_w + rhs._w, _x + rhs._x, _y + rhs._y, _z + rhs._z);
}

Quaternion Quaternion::operator-(const Quaternion &rhs) const
{
  return Quaternion(_w - rhs._w, _x - rhs._x, _y - rhs._y, _z - rhs._z);
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
  return Multiply(*this, rhs);
}

Quaternion& Quaternion::operator+=(float32 rhs)
{
  _w += rhs;
  _x += rhs;
  _y += rhs;
  _z += rhs;
  return *this;
}

Quaternion& Quaternion::operator-=(float32 rhs)
{
  _w -= rhs;
  _x -= rhs;
  _y -= rhs;
  _z -= rhs;
  return *this;
}

Quaternion& Quaternion::operator*=(float32 rhs)
{
  _w *= rhs;
  _x *= rhs;
  _y *= rhs;
  _z *= rhs;
  return *this;
}

Quaternion& Quaternion::operator+=(const Quaternion &rhs)
{
  _w += rhs._w;
  _x += rhs._x;
  _y += rhs._z;
  _z += rhs._z;
  return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion &rhs)
{
  _w -= rhs._w;
  _x -= rhs._x;
  _y -= rhs._z;
  _z -= rhs._z;
  return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
  _w *= _w * rhs._w - _x * rhs._x - _y * rhs._y - _z * rhs._z;
  _x *= _w * rhs._x + _x * rhs._w + _y * rhs._z - _z * rhs._y;
  _y *= _w * rhs._y + _y * rhs._w + _z * rhs._x - _x * rhs._z;
  _x *= _w * rhs._z + _z * rhs._w + _x * rhs._y - _y * rhs._x;
  return *this;
}

bool Quaternion::operator==(const Quaternion& rhs) const
{
  return _w == rhs._w && _x == rhs._x && _y == rhs._y && _z == rhs._z;
}

bool Quaternion::operator!=(const Quaternion& rhs) const
{
  return _w != rhs._w && _x != rhs._x && _y != rhs._y && _z != rhs._z;
}

float32 Quaternion::Norm() const
{
  return sqrtf(_x * _x + _y * _y + _z * _z + _w * _w);
}

void Quaternion::Normalize()
{
  float32 norm = Norm();
  float32 normInv = 1.0f / norm;
  _x *= normInv;
  _y *= normInv;
  _z *= normInv;
  _w *= normInv;
}

Vector3 Quaternion::Rotate(const Vector3& vec) const
{
  Matrix3 rot(*this);
  Vector3 result(vec);
  return rot * result;
}

Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float32 t)
{
  float32 d = Dot(a, b);
  float32 flip = d >= 0.0f ? 1.0f : -1.0f;
  
  Quaternion output = flip * (1.0f - t) * a + t * b;
  output.Normalize();
  return output;
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float32 t)
{
  float32 theta = (Math::ACos(Quaternion::Dot(a, b))).InRadians();
  float32 k = theta / Math::Sin(theta);
  float32 wa = Math::Sin(1.0f - t) * k;
  float32 wb = Math::Sin(t) * k;
  return wa * a + wb * b;
}

Quaternion operator+(float32 lhs, const Quaternion& rhs)
{
  return Quaternion(lhs + rhs._w, lhs + rhs._x, lhs + rhs._y, lhs + rhs._z);
}

Quaternion operator-(float32 lhs, const Quaternion& rhs)
{
  return Quaternion(lhs - rhs._w, lhs - rhs._x, lhs - rhs._y, lhs - rhs._z);
}

Quaternion operator*(float32 lhs, const Quaternion& rhs)
{
  return Quaternion(lhs * rhs._w, lhs * rhs._x, lhs * rhs._y, lhs * rhs._z);
}

Quaternion Quaternion::Multiply(const Quaternion& q1, const Quaternion& q2)
{
  float32 x =  q1._x * q2._w + q1._y * q2._z - q1._z * q2._y + q1._w * q2._x;
  float32 y = -q1._x * q2._z + q1._y * q2._w + q1._z * q2._x + q1._w * q2._y;
  float32 z =  q1._x * q2._y - q1._y * q2._x + q1._z * q2._w + q1._w * q2._z;
  float32 w = -q1._x * q2._x - q1._y * q2._y - q1._z * q2._z + q1._w * q2._w;
  return Quaternion(w, x, y, z);
}

void Quaternion::FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
{
  Radian halfXAngle = xAngle * 0.5f;
  Radian halfYAngle = yAngle * 0.5f;
  Radian halfZAngle = zAngle * 0.5f;
  
  float cx = Math::Cos(halfXAngle);
  float sx = Math::Sin(halfXAngle);
  
  float cy = Math::Cos(halfYAngle);
  float sy = Math::Sin(halfYAngle);
  
  float cz = Math::Cos(halfZAngle);
  float sz = Math::Sin(halfZAngle);
  
  Quaternion quatX(cx, sx, 0.0f, 0.0f);
  Quaternion quatY(cy, 0.0f, sy, 0.0f);
  Quaternion quatZ(cz, 0.0f, 0.0f, sz);
  
  *this = (quatZ * quatX) * quatY;
}

void Quaternion::FromAxisAngle(const Vector3& axis, const Radian& angle)
{
  Radian halfAngle = 0.5f * angle;
  float32 sin = Math::Sin(halfAngle);
  
  _w = Math::Cos(halfAngle);
  _z = sin * axis[2];
  _y = sin * axis[1];
  _x = sin * axis[0];
}
