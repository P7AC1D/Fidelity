#pragma once
#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "Component.h"

namespace Components
{
class PointLight : public Component
{
public:
  PointLight();
  PointLight(const Vector3& position,
             const Vector3& colour,
             float32 radius);

  inline void SetPosition(const Vector3& position) { _position = position; }
  inline void SetColour(const Vector3& colour) { _colour = colour; }
  inline void SetRadius(float32 radius) { _radius = radius; }

  inline Vector3 GetPosition() const { return _position; }
  inline Vector3 GetColour() const { return _colour; }
  inline float32 GetRadius() const { return _radius; }

private:
  Vector3 _position;
  Vector3 _colour;
  float32 _radius;
};
}