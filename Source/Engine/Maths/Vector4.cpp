#include "Vector4.hpp"

#include <cassert>
#include <cmath>

#include "Vector2.hpp"
#include "Vector3.hpp"

Vector4 Vector4::One = Vector4(1.0f);

float32 Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
{
  return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z + lhs.W * rhs.W;
}

Vector4 Vector4::Normalize(const Vector4& vec)
{
  float32 length = vec.Length();
  float32 lengthInv = 1.0f / length;
  return Vector4(vec[0] * lengthInv, vec[1] * lengthInv, vec[2] * lengthInv, vec[3] * lengthInv);
}

Vector4::Vector4()
{
  X = 0.0f;
  Y = 0.0f;
  Z = 0.0f;
  W = 0.0f;
}

Vector4::Vector4(float32 k)
{
  X = k;
  Y = k;
  Z = k;
  W = k;
}

Vector4::Vector4(float32 a, float32 b, float32 c, float32 d)
{
  X = a;
  Y = b;
  Z = c;
  W = d;
}

Vector4::Vector4(const Vector2& vec, float32 c, float32 d)
{
  X = vec[0];
  Y = vec[1];
  Z = c;
  W = d;
}

Vector4::Vector4(const Vector3& vec, float32 d)
{
  X = vec[0];
  Y = vec[1];
  Z = vec[2];
  W = d;
}

Vector4::Vector4(const Vector4& vec)
{
  X = vec[0];
  Y = vec[1];
  Z = vec[2];
  W = vec[3];
}

Vector4& Vector4::operator=(const Vector4& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
  Z = rhs.Z;
  W = rhs.W;
  return *this;
}

Vector4 Vector4::operator+(const Vector4 &rhs) const
{
  float32 x = X + rhs.X;
  float32 y = Y + rhs.Y;
  float32 z = Z + rhs.Z;
  float32 w = W + rhs.W;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator-(const Vector4 &rhs) const
{
  float32 x = X - rhs.X;
  float32 y = Y - rhs.Y;
  float32 z = Z - rhs.Z;
  float32 w = W - rhs.W;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator*(const Vector4& rhs) const
{
  float32 x = X * rhs.X;
  float32 y = Y * rhs.Y;
  float32 z = Z * rhs.Z;
  float32 w = W * rhs.W;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator+(float32 rhs) const
{
  float32 x = X + rhs;
  float32 y = Y + rhs;
  float32 z = Z + rhs;
  float32 w = W + rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator-(float32 rhs) const
{
  float32 x = X - rhs;
  float32 y = Y - rhs;
  float32 z = Z - rhs;
  float32 w = W - rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator*(float32 rhs) const
{
  float32 x = X * rhs;
  float32 y = Y * rhs;
  float32 z = Z * rhs;
  float32 w = W * rhs;
  return Vector4(x, y, z, w);
}

Vector4 Vector4::operator+=(const Vector4& rhs)
{
  X += rhs.X;
  Y += rhs.Y;
  Z += rhs.Z;
  W += rhs.W;
  return *this;
}

Vector4 Vector4::operator-=(const Vector4& rhs)
{
  X -= rhs.X;
  Y -= rhs.Y;
  Z -= rhs.Z;
  W -= rhs.W;
  return *this;
}

Vector4& Vector4::operator+=(float32 rhs)
{
  X += rhs;
  Y += rhs;
  Z += rhs;
  W += rhs;
  return *this;
}

Vector4& Vector4::operator-=(float32 rhs)
{
  X -= rhs;
  Y -= rhs;
  Z -= rhs;
  W -= rhs;
  return *this;
}

Vector4& Vector4::operator*=(float32 rhs)
{
  X *= rhs;
  Y *= rhs;
  Z *= rhs;
  W *= rhs;
  return *this;
}

bool Vector4::operator==(const Vector4& rhs) const
{
  return X == rhs.X && Y == rhs.Y && Z && rhs.Z && W == rhs.W;
}

bool Vector4::operator!=(const Vector4& rhs) const
{
  return X != rhs.X && Y != rhs.Y && Z != rhs.Z && W != rhs.W;
}

float32& Vector4::operator[](uint32 i)
{
  assert(i < 4);
  return *(&X + i);
}

const float32& Vector4::operator[](uint32 i) const
{
  assert(i < 4);
  return *(&X + i);
}

float32 Vector4::Length() const
{
  return sqrtf(X * X + Y * Y + Z * Z + W * W);
}

void Vector4::Normalize()
{
  float32 length = Length();
  float32 lengthInv = 1.0f / length;
  X *= lengthInv;
  Y *= lengthInv;
  Z *= lengthInv;
  W *= lengthInv;
}

const float32* Vector4::Ptr() const
{
  return &X;
}

Vector4 operator+(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs + rhs.X;
  float32 y = lhs + rhs.Y;
  float32 z = lhs + rhs.Z;
  float32 w = lhs + rhs.W;
  return Vector4(x, y, z, w);
}

Vector4 operator-(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs - rhs.X;
  float32 y = lhs - rhs.Y;
  float32 z = lhs - rhs.Z;
  float32 w = lhs - rhs.W;
  return Vector4(x, y, z, w);
}

Vector4 operator*(float32 lhs, const Vector4& rhs)
{
  float32 x = lhs * rhs.X;
  float32 y = lhs * rhs.Y;
  float32 z = lhs * rhs.Z;
  float32 w = lhs * rhs.W;
  return Vector4(x, y, z, w);
}
