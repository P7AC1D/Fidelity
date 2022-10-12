#pragma once
#include <cmath>

#include "../Core/Types.hpp"
#include "Degree.hpp"
#include "Radian.hpp"

class Vector3;
class Vector4;

class Math
{
public:
  static constexpr float32 Pi = 3.14159265358979323846f;
  static constexpr float32 TwoPi = (float32)(2.0f * Pi);
  static constexpr float32 HalfPi = (float32)(0.5f * Pi);
  static constexpr float32 Deg2Rad = Pi / 180.0f;
  static constexpr float32 Rad2Deg = 180.0f / Pi;

  static float32 Lerp(float32 a, float32 b, float32 t);

  static float32 InverseSqrt(float32 value);

  static float32 Sin(const Radian &angle);
  static float32 Cos(const Radian &angle);
  static float32 Tan(const Radian &angle);
  static float32 RoundToEven(float32 val);

  static Radian ASin(float32 value);
  static Radian ACos(float32 value);
  static Radian ATan(float32 value);
  static Radian ATan2(float32 x, float32 y);

  static float32 Clamp(float32 value, float32 min, float32 max);
  static float32 Abs(float32 value);

  static Vector3 Clamp(const Vector3 &a, float32 min, float32 max);
  static Vector3 Min(Vector3 a, Vector3 b);
  static Vector4 Min(Vector4 a, Vector4 b);
  static Vector3 Max(Vector3 a, Vector3 b);
  static Vector4 Max(Vector4 a, Vector4 b);
  static Vector3 Abs(Vector3 a);

  static Vector3 RoundToEven(const Vector3 &vec);
  static Vector4 RoundToEven(const Vector4 &vec);
};
