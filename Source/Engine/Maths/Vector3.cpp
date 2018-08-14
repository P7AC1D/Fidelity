#include "Vector3.hpp"

#include <cassert>
#include <cmath>

#include "Vector2.hpp"
#include "Vector4.hpp"

Vector3 Vector3::Zero = Vector3(0.0f);
Vector3 Vector3::Identity = Vector3(1.0f);

float32 Vector3::Length(const Vector3& a)
{
	return a.Length();
}

float32 Vector3::Dot(const Vector3& a, const Vector3& b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vector3 Vector3::Cross(const Vector3 &a, const Vector3 &b)
{
  return Vector3(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

Vector3 Vector3::Normalize(const Vector3& vec)
{
  float32 length = vec.Length();
  if (length == 0.0f)
  {
    return vec;
  }
  float32 lengthInv = 1.0f / length;
  return Vector3(vec[0] * lengthInv, vec[1] * lengthInv, vec[2] * lengthInv);
}

Vector3::Vector3()
{
  X = 0.0f;
  Y = 0.0f;
  Z = 0.0f;
}

Vector3::Vector3(float32 k)
{
  X = k;
  Y = k;
  Z = k;
}

Vector3::Vector3(float32 a, float32 b, float32 c)
{
  X = a;
  Y = b;
  Z = c;
}

Vector3::Vector3(const Vector2& vec, float32 k)
{
  X = vec[0];
  Y = vec[1];
  Z = k;
}

Vector3::Vector3(const Vector3& vec)
{
  X = vec.X;
  Y = vec.Y;
  Z = vec.Z;
}

Vector3::Vector3(const Vector4& vec)
{
	X = vec.X;
	Y = vec.Y;
	Z = vec.Z;
}

Vector3 Vector3::operator-() const
{
  return Vector3(-X, -Y, -Z);
}

Vector3& Vector3::operator=(const Vector3& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
  Z = rhs.Z;
  return *this;
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
  float32 x = X + rhs.X;
  float32 y = Y + rhs.Y;
  float32 z = Z + rhs.Z;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
  float32 x = X - rhs.X;
  float32 y = Y - rhs.Y;
  float32 z = Z - rhs.Z;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator+(float32 rhs) const
{
  float32 x = X + rhs;
  float32 y = Y + rhs;
  float32 z = Z + rhs;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator-(float32 rhs) const
{
  float32 x = X - rhs;
  float32 y = Y - rhs;
  float32 z = Z - rhs;
  return Vector3(x, y, z);
}

Vector3 Vector3::operator*(float32 rhs) const
{
  float32 x = X * rhs;
  float32 y = Y * rhs;
  float32 z = Z * rhs;
  return Vector3(x, y, z);
}

Vector3  Vector3::operator/(float32 rhs) const
{
	float32 x = X / rhs;
	float32 y = Y / rhs;
	float32 z = Z / rhs;
	return Vector3(x, y, z);
}

Vector3 Vector3::operator*(const Vector3& rhs) const
{
  float32 x = X * rhs[0];
  float32 y = Y * rhs[1];
  float32 z = Z * rhs[2];
  return Vector3(x, y, z);
}

Vector3& Vector3::operator+=(const Vector3& rhs)
{
  X += rhs.X;
  Y += rhs.Y;
  Z += rhs.Z;
  return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
  X -= rhs.X;
  Y -= rhs.Y;
  Z -= rhs.Z;
  return *this;
}

Vector3& Vector3::operator+=(float32 rhs)
{
  X += rhs;
  Y += rhs;
  Z += rhs;
  return *this;
}

Vector3& Vector3::operator-=(float32 rhs)
{
  X -= rhs;
  Y -= rhs;
  Z -= rhs;
  return *this;
}

Vector3& Vector3::operator*=(float32 rhs)
{
  X *= rhs;
  Y *= rhs;
  Z *= rhs;
  return *this;
}

Vector3& Vector3::operator*=(const Vector3& rhs)
{
	X *= rhs.X;
	Y *= rhs.Y;
	Z *= rhs.Z;
	return *this;
}

bool Vector3::operator==(const Vector3& rhs) const
{
  return X == rhs.X && Y == rhs.Y && Z == rhs.Z;
}

bool Vector3::operator!=(const Vector3& rhs) const
{
  return !(*this == rhs);
}

float32& Vector3::operator[](uint32 i)
{
  assert(i < 3);
  return *(&X + i);
}

const float32& Vector3::operator[](uint32 i) const
{
  assert(i < 3);
  return *(&X + i);
}

float32 Vector3::Length() const
{
  return sqrtf(X * X + Y * Y + Z * Z);
}

void Vector3::Normalize()
{
  float32 length = this->Length();
  float32 lengthInv = 1.0f / length;
  X *= lengthInv;
  Y *= lengthInv;
  Z *= lengthInv;
}

const float32* Vector3::Ptr() const
{
  return &X;
}

Vector3 operator+(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs + rhs.X;
  float32 y = lhs + rhs.Y;
  float32 z = lhs + rhs.Z;
  return Vector3(x, y, z);
}

Vector3 operator-(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs - rhs.X;
  float32 y = lhs - rhs.Y;
  float32 z = lhs - rhs.Z;
  return Vector3(x, y, z);
}

Vector3 operator*(float32 lhs, const Vector3& rhs)
{
  float32 x = lhs * rhs.X;
  float32 y = lhs * rhs.Y;
  float32 z = lhs * rhs.Z;
  return Vector3(x, y, z);
}
