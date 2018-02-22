#include "Vector2I.hpp"

#include <cassert>
#include <cmath>

Vector2i Vector2i::Identity = Vector2i(1);
Vector2i Vector2i::Zero = Vector2i(0);

Vector2i::Vector2i()
{
  X = 0;
  Y = 0;
}

Vector2i::Vector2i(int32 a)
{
  X = a;
  Y = a;
}

Vector2i::Vector2i(int32 x, int32 y)
{
  X = x;
  Y = y;
}

Vector2i& Vector2i::operator=(const Vector2i& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
  return *this;
}

Vector2i::Vector2i(const Vector2i& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
}

Vector2i Vector2i::operator+(const Vector2i& rhs)
{
  return Vector2i(X + rhs.X, Y + rhs.Y);
}

Vector2i Vector2i::operator-(const Vector2i& rhs)
{
  return Vector2i(X - rhs.X, Y - rhs.Y);
}

Vector2i Vector2i::operator+(int32 rhs)
{
  return Vector2i(X + rhs, Y + rhs);
}

Vector2i Vector2i::operator-(int32 rhs)
{
  return Vector2i(X - rhs, Y - rhs);
}

Vector2i Vector2i::operator*(int32 rhs)
{
  return Vector2i(rhs * X, rhs * Y);
}

Vector2i& Vector2i::operator+=(const Vector2i& rhs)
{
  X += rhs.X;
  Y += rhs.Y;
  return *this;
}

Vector2i& Vector2i::operator-=(const Vector2i& rhs)
{
  X -= rhs.X;
  Y -= rhs.Y;
  return *this;
}

Vector2i& Vector2i::operator-=(int32 rhs)
{
  X -= rhs;
  Y -= rhs;
  return *this;
}

Vector2i& Vector2i::operator+=(int32 rhs)
{
  X += rhs;
  Y += rhs;
  return *this;
}

Vector2i& Vector2i::operator*=(int32 rhs)
{
  X *= rhs;
  Y *= rhs;
  return *this;
}

bool Vector2i::operator==(const Vector2i& rhs) const
{
  return X == rhs.X && Y == rhs.Y;
}

bool Vector2i::operator!=(const Vector2i& rhs) const
{
  return X != rhs.X && Y != rhs.Y;
}

int32& Vector2i::operator[](uint32 i)
{
  assert(i < 2);
  return *(&X + i);
}

int32 Vector2i::operator[](uint32 i) const
{
  assert(i < 2);
  return *(&X + i);
}

const int32* Vector2i::Ptr() const
{
  return &X;
}

Vector2i operator+(int32 a, const Vector2i& b)
{
  return Vector2i(a + b.X, a + b.Y);
}

Vector2i operator-(int32 a, const Vector2i& b)
{
  return Vector2i(a - b.X, a - b.Y);
}

Vector2i operator*(int32 a, const Vector2i& b)
{
  return Vector2i(a * b.X, a * b.Y);
}
