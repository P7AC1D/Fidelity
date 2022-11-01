#pragma once

#include "../Core/Component.h"
#include "../Core/ComponentBuilder.hpp"
#include "../Core/Types.hpp"
#include "../Core/Maths.h"
#include "Light.h"

class LightBuilder : public ComponentBuilder
{
public:
  LightBuilder();

  LightBuilder &withColour(const Colour &colour);
  LightBuilder &withRadius(float32 radius);
  LightBuilder &withLightType(LightType lightType);
  LightBuilder &withIntensity(float32 intensity);

  std::shared_ptr<Component> build() override;

private:
  Colour _colour;
  float32 _radius;
  LightType _lightType;
  float32 _intensity;
};