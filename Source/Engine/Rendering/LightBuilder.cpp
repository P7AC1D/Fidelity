#include "LightBuilder.hpp"

LightBuilder::LightBuilder() : _colour(Colour::White),
                               _radius(10.f),
                               _lightType(LightType::Point),
                               _intensity(10.f)
{
}

LightBuilder &LightBuilder::withColour(const Colour &colour)
{
  _colour = colour;
  return *this;
}

LightBuilder &LightBuilder::withRadius(float32 radius)
{
  _radius = radius;
  return *this;
}

LightBuilder &LightBuilder::withLightType(LightType lightType)
{
  _lightType = lightType;
  return *this;
}

LightBuilder &LightBuilder::withIntensity(float32 intensity)
{
  _intensity = intensity;
  return *this;
}

std::shared_ptr<Component> LightBuilder::build()
{
  std::shared_ptr<Light> light(new Light());
  light->setColour(_colour);
  light->setIntensity(_intensity);
  light->setLightType(_lightType);
  light->setRadius(_radius);
  return light;
}
