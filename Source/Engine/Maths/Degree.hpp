#pragma once
#include "../Core/Types.hpp"

class Radian;

class Degree
{
public:
  Degree(float32 deg = 0.0f);
  Degree(const Degree& deg);
  Degree(const Radian& rad);
  
  Degree& operator=(const Radian& r);
  Degree& operator=(const Degree& d);
  
  Degree operator+(const Radian& r) const;
  Degree operator+(const Degree& d) const;
  Degree& operator+=(const Radian& r);
  Degree& operator+=(const Degree& d);
  
  Degree operator-(const Radian& r) const;
  Degree operator-(const Degree& d) const;
  Degree& operator-=(const Radian& r);
  Degree& operator-=(const Degree& d);
  
  Degree operator*(const Radian& r) const;
  Degree operator*(const Degree& d) const;
  Degree& operator*=(const Radian& r);
  Degree& operator*=(const Degree& d);
  
  float32 InDegrees() const;
  float32 InRadians() const;
  
private:
  float32 _deg;
};
