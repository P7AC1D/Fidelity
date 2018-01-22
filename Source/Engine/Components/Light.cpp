#include "Light.h"

std::shared_ptr<Light> Light::CreatePoint(const Vector3& colour, const Vector3& position, float32 radius)
{
  return std::make_shared<Light>(LightType::Point, colour, Vector3::Zero, position, radius);
}

std::shared_ptr<Light> Light::CreateDirectional(const Vector3& colour, const Vector3& direction)
{
  return std::make_shared<Light>(LightType::Directional, colour, direction, Vector3::Zero, 0.0f);
}

Light::Light(LightType lightType, const Vector3& colour, const Vector3& direction, const Vector3& position, float32 radius):
  _radius(radius),
  _colour(colour),
  _position(position),
  _direction(direction),
  _lightType(lightType)
{
}