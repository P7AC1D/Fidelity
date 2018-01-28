#pragma once
#include <cmath>

#include "../Core/Types.hpp"

class Degree;
class Radian;

class Math
{
public:
  static constexpr float32 Pi = 3.14159265358979323846f;
  static constexpr float32 TwoPi = (float32)(2.0f * Pi);
  static constexpr float32 HalfPi = (float32)(0.5f * Pi);
  static constexpr float32 Deg2Rad = Pi / 180.0f;
  static constexpr float32 Rad2Deg = 180.0f / Pi;
  
  static float32 Sin(const Radian& angle);
  static float32 Cos(const Radian& angle);
  static float32 Tan(const Radian& angle);

  static Radian ASin(float32 value);
  static Radian ACos(float32 value);
  static Radian ATan(float32 value);

  static float32 Clamp(float32 value, float32 min, float32 max);
  static float32 Abs(float32 value);
};
