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

float32 Math::ASin(const Radian& angle)
{
  return std::asinf(angle.InRadians());
}

float32 Math::ACos(const Radian& angle)
{
  return std::acosf(angle.InRadians());
}
