#include "Vector2I.hpp"

#include <cassert>
#include <cmath>

Vector2I Vector2I::Identity = Vector2I(1);
Vector2I Vector2I::Zero = Vector2I(0);

Vector2I::Vector2I()
{
  X = 0;
  Y = 0;
}

Vector2I::Vector2I(int32 a)
{
  X = a;
  Y = a;
}

Vector2I::Vector2I(int32 x, int32 y)
{
  X = x;
  Y = y;
}

Vector2I& Vector2I::operator=(const Vector2I& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
  return *this;
}

Vector2I::Vector2I(const Vector2I& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
}

Vector2I Vector2I::operator+(const Vector2I& rhs)
{
  return Vector2I(X + rhs.X, Y + rhs.Y);
}

Vector2I Vector2I::operator-(const Vector2I& rhs)
{
  return Vector2I(X - rhs.X, Y - rhs.Y);
}

Vector2I Vector2I::operator+(int32 rhs)
{
  return Vector2I(X + rhs, Y + rhs);
}

Vector2I Vector2I::operator-(int32 rhs)
{
  return Vector2I(X - rhs, Y - rhs);
}

Vector2I Vector2I::operator*(int32 rhs)
{
  return Vector2I(rhs * X, rhs * Y);
}

Vector2I& Vector2I::operator+=(const Vector2I& rhs)
{
  X += rhs.X;
  Y += rhs.Y;
  return *this;
}

Vector2I& Vector2I::operator-=(const Vector2I& rhs)
{
  X -= rhs.X;
  Y -= rhs.Y;
  return *this;
}

Vector2I& Vector2I::operator-=(int32 rhs)
{
  X -= rhs;
  Y -= rhs;
  return *this;
}

Vector2I& Vector2I::operator+=(int32 rhs)
{
  X += rhs;
  Y += rhs;
  return *this;
}

Vector2I& Vector2I::operator*=(int32 rhs)
{
  X *= rhs;
  Y *= rhs;
  return *this;
}

bool Vector2I::operator==(const Vector2I& rhs) const
{
  return X == rhs.X && Y == rhs.Y;
}

bool Vector2I::operator!=(const Vector2I& rhs) const
{
  return X != rhs.X && Y != rhs.Y;
}

int32& Vector2I::operator[](uint32 i)
{
  assert(i < 2);
  return *(&X + i);
}

int32 Vector2I::operator[](uint32 i) const
{
  assert(i < 2);
  return *(&X + i);
}

const int32* Vector2I::Ptr() const
{
  return &X;
}

Vector2I operator+(int32 a, const Vector2I& b)
{
  return Vector2I(a + b.X, a + b.Y);
}

Vector2I operator-(int32 a, const Vector2I& b)
{
  return Vector2I(a - b.X, a - b.Y);
}

Vector2I operator*(int32 a, const Vector2I& b)
{
  return Vector2I(a * b.X, a * b.Y);
}
