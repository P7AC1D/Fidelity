#pragma once
#include "../Core/Types.hpp"

class Degree;

class Radian
{
public:
  Radian(float32 rad = 0.0f);
  Radian(const Radian& rad);
  Radian(const Degree& deg);
  
  Radian& operator=(const Radian& r);
  Radian& operator=(const Degree& d);
  
  Radian operator+(float32 f) const;
  Radian operator+(const Radian& r) const;
  Radian operator+(const Degree& d) const;
  Radian& operator+=(float32 f);
  Radian& operator+=(const Radian& r);
  Radian& operator+=(const Degree& d);
  
  Radian operator-(float32 f) const;
  Radian operator-(const Radian& r) const;
  Radian operator-(const Degree& d) const;
  Radian& operator-=(float32 f);
  Radian& operator-=(const Radian& r);
  Radian& operator-=(const Degree& d);
  
  Radian operator*(float32 f) const;
  Radian operator*(const Radian& r) const;
  Radian operator*(const Degree& d) const;
  Radian& operator*=(float32 f);
  Radian& operator*=(const Radian& r);
  Radian& operator*=(const Degree& d);
  
  float32 InDegrees() const;
  float32 InRadians() const;
  
  friend Radian operator+(float32 lhs, const Radian& rhs);
  friend Radian operator-(float32 lhs, const Radian& rhs);
  friend Radian operator*(float32 lhs, const Radian& rhs);
  
private:
  float32 _rad;
};
