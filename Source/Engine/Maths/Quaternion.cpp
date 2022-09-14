#include "Quaternion.hpp"

#include <algorithm>
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
  if (norm <= 0.0f)
  {
    return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
  }    
  float32 normInv = 1.0f / norm;
  return Quaternion(quat[0] * normInv, quat[1] * normInv, quat[2] * normInv, quat[3] * normInv);
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, const Radian& angle)
{
  Radian halfAngle = 0.5f * angle;
  float32 sin = Math::Sin(halfAngle);

  float32 W = Math::Cos(halfAngle);
  float32 Z = sin * axis.Z;
  float32 Y = sin * axis.Y;
  float32 X = sin * axis.X;
  return Quaternion(W, X, Y, Z);
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
  FromRotationMatrix(rotMax);
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

Quaternion& Quaternion::operator=(const Matrix3& rotMat)
{
  FromRotationMatrix(rotMat);
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

Quaternion Quaternion::operator/(float32 rhs) const
{
  return Quaternion(W / rhs, X / rhs, Y / rhs, Z / rhs);
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
  return Quaternion(*this) *= rhs;
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

Quaternion& Quaternion::operator*=(const Quaternion& q)
{
  Quaternion p(*this);
  this->W = p.W * q.W - p.X * q.X - p.Y * q.Y - p.Z * q.Z;
  this->X = p.W * q.X + p.X * q.W + p.Y * q.Z - p.Z * q.Y;
  this->Y = p.W * q.Y + p.Y * q.W + p.Z * q.X - p.X * q.Z;
  this->Z = p.W * q.Z + p.Z * q.W + p.X * q.Y - p.Y * q.X;
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

Quaternion Quaternion::Conjugate() const
{
  return Quaternion(W, -X, -Y, -Z);
}

Quaternion Quaternion::Inverse() const
{
  return Conjugate() / Dot(Quaternion(*this), Quaternion(*this));
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

Vector3 Quaternion::ToEuler() const
{
  return Vector3(Radian(Pitch()).InDegrees(), Radian(Yaw()).InDegrees(), Radian(Roll()).InDegrees());
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

void Quaternion::FromEulerAngles(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
{
  FromAxisAngle(Vector3(0.0, 0.0, 1.0), zAngle);
  FromAxisAngle(Vector3(1.0, 0.0, 0.0), xAngle);
  FromAxisAngle(Vector3(0.0, 1.0, 0.0), yAngle);
}

void Quaternion::FromRotationMatrix(const Matrix3 & m)
{
  float32 fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
  float32 fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
  float32 fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
  float32 fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

  int biggestIndex = 0;
  float32 fourBiggestSquaredMinus1 = fourWSquaredMinus1;
  if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourXSquaredMinus1;
    biggestIndex = 1;
  }
  if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourYSquaredMinus1;
    biggestIndex = 2;
  }
  if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourZSquaredMinus1;
    biggestIndex = 3;
  }

  float32 biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
  float32 mult = 0.25f / biggestVal;

  switch (biggestIndex)
  {
    case 0:
      *this = Quaternion(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);
      break;
    case 1:
      *this = Quaternion((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);
      break;
    case 2:
      *this = Quaternion((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);
      break;
    case 3:
      *this = Quaternion((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);
      break;
    default: 
      *this = Quaternion(1, 0, 0, 0);
  }
}

float32 Quaternion::Pitch() const
{
	const float32 y = 2.0f * (Y * Z + W * X);
	const float32 x = W * W - X * X - Y * Y + Z * Z;

	if (y == 0.0f && x == 0.0f)
	{
    return 2.0f * std::atan2(X, W);		
	}
	return std::atan2(y, x);
}

float32 Quaternion::Roll() const
{
  return std::atan2(2.0f * (X * Y + W * Z), W * W + X * X - Y * Y - Z * Z);
}

float32 Quaternion::Yaw() const
{
  return std::asin(Math::Clamp(-2.0f * (X * Z - W * Y), -1.0f, 1.0f));
}