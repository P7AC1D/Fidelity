#include "PointLight.h"

namespace Components
{
PointLight::PointLight() :
  _position(0.0f),
  _colour(1.0f),
  _radius(1.0f),
  Component("PointLight")
{
}

PointLight::PointLight(const Vector3& position,
                       const Vector3& colour,
                       float32 radius) :
  _position(position),
  _colour(colour),
  _radius(radius),
  Component("PointLight")
{
}
}
