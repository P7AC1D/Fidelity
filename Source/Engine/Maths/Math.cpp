#include "Math.hpp"

#include "Radian.hpp"

float32 Math::Sin(const Radian& angle)
{
  return std::sinf(angle.InRadians());
}

float32 Math::Cos(const Radian& angle)
{
  return std::cosf(angle.InRadians());
}

float32 Math::Tan(const Radian& angle)
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

float32 Math::Clamp(float32 value, float32 min, float32 max)
{
  return value < min ? min : value > max ? max : value;
}