#include "Radian.hpp"

#include "Degree.hpp"
#include "Math.hpp"

Radian::Radian(float32 rad) : _rad(rad)
{
}

Radian::Radian(const Radian &rad) : _rad(rad._rad)
{
}

Radian::Radian(const Degree &deg) : _rad(deg.InRadians())
{
}

Radian &Radian::operator=(const Radian &r)
{
  _rad = r.InRadians();
  return *this;
}

Radian &Radian::operator=(const Degree &d)
{
  _rad = d.InRadians();
  return *this;
}

Radian Radian::operator+(float32 f) const
{
  return Radian(f + _rad);
}

Radian Radian::operator+(const Radian &r) const
{
  return Radian(_rad + r._rad);
}

Radian Radian::operator+(const Degree &d) const
{
  return Radian(_rad + d.InRadians());
}

Radian &Radian::operator+=(float32 f)
{
  _rad += f;
  return *this;
}

Radian &Radian::operator+=(const Radian &r)
{
  _rad += r._rad;
  return *this;
}

Radian &Radian::operator+=(const Degree &d)
{
  _rad += d.InRadians();
  return *this;
}

Radian Radian::operator-(float32 f) const
{
  return Radian(_rad - f);
}

Radian Radian::operator-(const Radian &r) const
{
  return Radian(_rad - r._rad);
}

Radian Radian::operator-(const Degree &d) const
{
  return Radian(_rad - d.InRadians());
}

Radian &Radian::operator-=(float32 f)
{
  _rad -= f;
  return *this;
}

Radian &Radian::operator-=(const Radian &r)
{
  _rad -= r._rad;
  return *this;
}

Radian &Radian::operator-=(const Degree &d)
{
  _rad -= d.InRadians();
  return *this;
}

Radian Radian::operator*(float32 f) const
{
  return Radian(f * _rad);
}

Radian Radian::operator*(const Radian &r) const
{
  return Radian(_rad * r._rad);
}

Radian Radian::operator*(const Degree &d) const
{
  return Radian(_rad * d.InRadians());
}

Radian &Radian::operator*=(float32 f)
{
  _rad *= f;
  return *this;
}

Radian &Radian::operator*=(const Radian &r)
{
  _rad *= r._rad;
  return *this;
}

Radian &Radian::operator*=(const Degree &d)
{
  _rad *= d.InRadians();
  return *this;
}

float32 Radian::InRadians() const
{
  return _rad;
}

float32 Radian::InDegrees() const
{
  return Math::Rad2Deg * _rad;
}

Radian operator+(float32 lhs, const Radian &rhs)
{
  return Radian(lhs + rhs._rad);
}

Radian operator-(float32 lhs, const Radian &rhs)
{
  return Radian(lhs - rhs._rad);
}

Radian operator*(float32 lhs, const Radian &rhs)
{
  return Radian(lhs * rhs._rad);
}

bool Radian::operator==(const Radian &rhs) const
{
  return _rad == rhs._rad;
}

bool Radian::operator!=(const Radian &rhs) const
{
  return _rad != rhs._rad;
}