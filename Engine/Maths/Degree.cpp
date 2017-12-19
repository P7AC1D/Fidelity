#include "Degree.hpp"

#include "Math.hpp"
#include "Radian.hpp"

Degree::Degree(float32 deg):
_deg(deg)
{
}

Degree::Degree(const Degree& deg):
_deg(deg.InDegrees())
{
}

Degree::Degree(const Radian& rad):
_deg(rad.InDegrees())
{
}

Degree& Degree::operator=(const Radian& r)
{
  _deg = r.InDegrees();
  return *this;
}

Degree& Degree::operator=(const Degree& d)
{
  _deg = d._deg;
  return *this;
}

Degree Degree::operator+(const Degree &d) const
{
  return Degree(_deg + d._deg);
}

Degree& Degree::operator+=(const Radian& r)
{
  _deg += r.InDegrees();
  return *this;
}

Degree& Degree::operator+=(const Degree& d)
{
  _deg += d._deg;
  return *this;
}

Degree Degree::operator-(const Radian& r) const
{
  return Degree(_deg - r.InDegrees());
}

Degree Degree::operator-(const Degree& d) const
{
  return Degree(_deg - d._deg);
}

Degree& Degree::operator-=(const Radian& r)
{
  _deg -= r.InDegrees();
  return *this;
}

Degree& Degree::operator-=(const Degree& d)
{
  _deg -= d._deg;
  return *this;
}

Degree Degree::operator*(const Radian& r) const
{
  return Degree(_deg * r.InDegrees());
}

Degree Degree::operator*(const Degree& d) const
{
  return Degree(_deg * d._deg);
}

Degree& Degree::operator*=(const Radian& r)
{
  _deg *= r.InDegrees();
  return *this;
}

Degree& Degree::operator*=(const Degree& d)
{
  _deg *= d._deg;
  return *this;
}

float32 Degree::InDegrees() const
{
  return _deg;
}

float32 Degree::InRadians() const
{
  return _deg * Math::Deg2Rad;
}
