#include "Vector2.hpp"

#include <cassert>
#include <cmath>

Vector2 Vector2::Identity = Vector2(1.0f);

float32 Vector2::Dot(const Vector2& a, const Vector2& b)
{
  return a[0] * b[0] + a[1] * b[1];
}

Vector2 Vector2::Normalize(const Vector2& vec)
{
  float32 length = vec.Length();
  float32 lengthInv = 1.0f / length;
  return Vector2(vec[0] * lengthInv, vec[1] * lengthInv);
}

Vector2::Vector2()
{
  X = 0.0f;
  Y = 0.0f;
}

Vector2::Vector2(float32 a)
{
  X = a;
  Y = a;
}

Vector2::Vector2(float32 x, float32 y)
{
  X = x;
  Y = y;
}

Vector2& Vector2::operator=(const Vector2& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
  return *this;
}

Vector2::Vector2(const Vector2& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
}

Vector2 Vector2::operator+(const Vector2& rhs) const
{
  return Vector2(X + rhs.X, Y + rhs.Y);
}

Vector2 Vector2::operator-(const Vector2& rhs) const
{
  return Vector2(X - rhs.X, Y - rhs.Y);
}

Vector2 Vector2::operator+(float32 rhs) const
{
  return Vector2(X + rhs, Y + rhs);
}

Vector2 Vector2::operator-(float32 rhs) const
{
  return Vector2(X - rhs, Y - rhs);
}

Vector2 Vector2::operator*(float32 rhs) const
{
  return Vector2(rhs * X, rhs * Y);
}

Vector2& Vector2::operator+=(const Vector2& rhs)
{
  X += rhs.X;
  Y += rhs.Y;
  return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
  X -= rhs.X;
  Y -= rhs.Y;
  return *this;
}

Vector2& Vector2::operator-=(float32 rhs)
{
  X -= rhs;
  Y -= rhs;
  return *this;
}

Vector2& Vector2::operator+=(float32 rhs)
{
  X += rhs;
  Y += rhs;
  return *this;
}

Vector2& Vector2::operator*=(float32 rhs)
{
  X *= rhs;
  Y *= rhs;
  return *this;
}

bool Vector2::operator==(const Vector2& rhs) const
{
  return X == rhs.X && Y == rhs.Y;
}

bool Vector2::operator!=(const Vector2& rhs) const
{
  return X != rhs.X && Y != rhs.Y;
}

float32& Vector2::operator[](uint32 i)
{
  assert(i < 2);
  return *(&X + i);
}

float32 Vector2::operator[](uint32 i) const
{
  assert(i < 2);
  return *(&X + i);
}

float32 Vector2::Length() const
{
  return sqrtf(X * X + Y * Y);
}

void Vector2::Normalize()
{
  float32 length = Length();
  float32 lengthInv = 1.0f / length;
  X *= lengthInv;
  Y *= lengthInv;
}

const float32* Vector2::Ptr() const
{
  return &X;
}

Vector2 operator+(float32 a, const Vector2& b)
{
  return Vector2(a + b.X, a + b.Y);
}

Vector2 operator-(float32 a, const Vector2& b)
{
  return Vector2(a - b.X, a - b.Y);
}

Vector2 operator*(float32 a, const Vector2& b)
{
  return Vector2(a * b.X, a * b.Y);
}
