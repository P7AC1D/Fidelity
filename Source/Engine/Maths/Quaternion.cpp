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
  return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z + lhs.W * rhs.W;
}

Quaternion Quaternion::Normalize(const Quaternion& quat)
{
  float32 norm = quat.Norm();
  float32 normInv = 1.0f / norm;
  return Quaternion(quat[0] * normInv, quat[1] * normInv, quat[2] * normInv, quat[3] * normInv);
}

Quaternion::Quaternion():
X(0.0f),
Y(0.0f),
Z(0.0f),
W(0.0f)
{
}

Quaternion::Quaternion(float32 _w, float32 _x, float32 _y, float32 _z):
X(_x),
Y(_y),
Z(_z),
W(_w)
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
  return *(&X + i);
}

float32 Quaternion::operator[](uint32 i) const
{
  assert(i < 4);
  return *(&X + i);
}

Quaternion& Quaternion::operator=(const Quaternion& rhs)
{
  W = rhs.W;
  Z = rhs.Z;
  Y = rhs.Y;
  X = rhs.X;
  return *this;
}

Quaternion Quaternion::operator+(float32 rhs) const
{
  return Quaternion(W + rhs, X + rhs, Y + rhs, Z + rhs);
}

Quaternion Quaternion::operator-(float32 rhs) const
{
  return Quaternion(W - rhs, X - rhs, Y - rhs, Z - rhs);
}

Quaternion Quaternion::operator*(float32 rhs) const
{
  return Quaternion(W * rhs, X * rhs, Y * rhs, Z * rhs);
}

Quaternion Quaternion::operator+(const Quaternion &rhs) const
{
  return Quaternion(W + rhs.W, X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

Quaternion Quaternion::operator-(const Quaternion &rhs) const
{
  return Quaternion(W - rhs.W, X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
  return Multiply(*this, rhs);
}

Quaternion& Quaternion::operator+=(float32 rhs)
{
  W += rhs;
  X += rhs;
  Y += rhs;
  Z += rhs;
  return *this;
}

Quaternion& Quaternion::operator-=(float32 rhs)
{
  W -= rhs;
  X -= rhs;
  Y -= rhs;
  Z -= rhs;
  return *this;
}

Quaternion& Quaternion::operator*=(float32 rhs)
{
  W *= rhs;
  X *= rhs;
  Y *= rhs;
  Z *= rhs;
  return *this;
}

Quaternion& Quaternion::operator+=(const Quaternion &rhs)
{
  W += rhs.W;
  X += rhs.X;
  Y += rhs.Z;
  Z += rhs.Z;
  return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion &rhs)
{
  W -= rhs.W;
  X -= rhs.X;
  Y -= rhs.Z;
  Z -= rhs.Z;
  return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
  W *= W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z;
  X *= W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y;
  Y *= W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z;
  X *= W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X;
  return *this;
}

bool Quaternion::operator==(const Quaternion& rhs) const
{
  return W == rhs.W && X == rhs.X && Y == rhs.Y && Z == rhs.Z;
}

bool Quaternion::operator!=(const Quaternion& rhs) const
{
  return W != rhs.W && X != rhs.X && Y != rhs.Y && Z != rhs.Z;
}

float32 Quaternion::Norm() const
{
  return sqrtf(X * X + Y * Y + Z * Z + W * W);
}

void Quaternion::Normalize()
{
  float32 norm = Norm();
  float32 normInv = 1.0f / norm;
  X *= normInv;
  Y *= normInv;
  Z *= normInv;
  W *= normInv;
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
  return Quaternion(lhs + rhs.W, lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z);
}

Quaternion operator-(float32 lhs, const Quaternion& rhs)
{
  return Quaternion(lhs - rhs.W, lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z);
}

Quaternion operator*(float32 lhs, const Quaternion& rhs)
{
  return Quaternion(lhs * rhs.W, lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
}

Quaternion Quaternion::Multiply(const Quaternion& q1, const Quaternion& q2)
{
  float32 x =  q1.X * q2.W + q1.Y * q2.Z - q1.Z * q2.Y + q1.W * q2.X;
  float32 y = -q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X + q1.W * q2.Y;
  float32 z =  q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W + q1.W * q2.Z;
  float32 w = -q1.X * q2.X - q1.Y * q2.Y - q1.Z * q2.Z + q1.W * q2.W;
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
  
  W = Math::Cos(halfAngle);
  Z = sin * axis[2];
  Y = sin * axis[1];
  X = sin * axis[0];
}
