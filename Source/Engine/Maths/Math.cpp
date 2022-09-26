#include "Math.hpp"

#include "Vector3.hpp"
#include "Vector4.hpp"

float32 Math::InverseSqrt(float32 value)
{
  union
  {
    float32 f;
    uint32 i;
  } conv;

  float32 x2;
  const float32 threehalfs = 1.5F;

  x2 = value * 0.5F;
  conv.f = value;
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f = conv.f * (threehalfs - (x2 * conv.f * conv.f));
  return conv.f;
}

float32 Math::Sin(const Radian &angle)
{
  return std::sinf(angle.InRadians());
}

float32 Math::Cos(const Radian &angle)
{
  return std::cosf(angle.InRadians());
}

float32 Math::Tan(const Radian &angle)
{
  return std::tanf(angle.InRadians());
}

Radian Math::ASin(float32 value)
{
  return Radian(std::asinf(value));
}

Radian Math::ACos(float32 value)
{
  return Radian(std::acosf(value));
}

Radian Math::ATan(float32 value)
{
  return Radian(std::atanf(value));
}

Radian Math::ATan2(float32 x, float32 y)
{
  return Radian(std::atan2f(x, y));
}

float32 Math::Clamp(float32 value, float32 min, float32 max)
{
  return value < min ? min : value > max ? max
                                         : value;
}

float32 Math::Abs(float32 value)
{
  return std::fabs(value);
}

Vector3 Math::Min(Vector3 a, Vector3 b)
{
  return Vector3(std::fmin(a.X, b.X), std::fmin(a.Y, b.Y), std::fmin(a.Z, b.Z));
}

Vector4 Math::Min(Vector4 a, Vector4 b)
{
  return Vector4(std::fmin(a.X, b.X), std::fmin(a.Y, b.Y), std::fmin(a.Z, b.Z), std::fmin(a.W, b.W));
}

Vector3 Math::Max(Vector3 a, Vector3 b)
{
  return Vector3(std::fmax(a.X, b.X), std::fmax(a.Y, b.Y), std::fmax(a.Z, b.Z));
}

Vector4 Math::Max(Vector4 a, Vector4 b)
{
  return Vector4(std::fmax(a.X, b.X), std::fmax(a.Y, b.Y), std::fmax(a.Z, b.Z), std::fmax(a.W, b.W));
}

Vector3 Math::Abs(Vector3 a)
{
  return Vector3(std::fabs(a.X), std::fabs(a.Y), std::fabs(a.Z));
}