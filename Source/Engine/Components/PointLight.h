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
             const Vector3& diffuseColour,
             const Vector3& specularColour,
             float32 constContribution,
             float32 linearContribition,
             float32 quadContribution);

  void SetPosition(const Vector3& position);
  void DiffuseColour(const Vector3& diffuseColour);
  void SpecularColour(const Vector3& specularColour);
  void ConstContrib(float32 constContribution);
  void LinearContrib(float32 linearContribution);
  void QuadraticContrib(float32 quadContribution);

  Vector3 GetPosition();
  Vector3 GetDiffuseColour();
  Vector3 GetSpecularColour();
  float32 GetConstContrib();
  float32 GetLinearContrib();
  float32 GetQuadraticContrib();

private:
  Vector3 _position;
  Vector3 _diffuseColour;
  Vector3 _specularColour;
  float32 _constContribution;
  float32 _linearContribution;
  float32 _quadraticContribution;
};
}