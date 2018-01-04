#include "PointLight.h"

namespace Components
{
PointLight::PointLight() :
  _position(0.0f),
  _diffuseColour(0.0f),
  _specularColour(0.0f),
  _constContribution(0.0f),
  _linearContribution(0.0f),
  _quadraticContribution(0.0f),
  Component("PointLight")
{
}

PointLight::PointLight(const Vector3& position,
                       const Vector3& diffuseColour,
                       const Vector3& specularColour,
                       float32 constContribution,
                       float32 linearContribition,
                       float32 quadContribution) :
  _position(position),
  _diffuseColour(diffuseColour),
  _specularColour(specularColour),
  _constContribution(constContribution),
  _linearContribution(linearContribition),
  _quadraticContribution(quadContribution),
  Component("PointLight")
{
}

void PointLight::SetPosition(const Vector3& position)
{
  _position = position;
}

void PointLight::DiffuseColour(const Vector3& diffuseColour)
{
  _diffuseColour = diffuseColour;
}

void PointLight::SpecularColour(const Vector3& specularColour)
{
  _specularColour = specularColour;
}

void PointLight::ConstContrib(float32 constContribution)
{
  _constContribution = constContribution;
}

void PointLight::LinearContrib(float32 linearContribution)
{
  _linearContribution = linearContribution;
}

void PointLight::QuadraticContrib(float32 quadContribution)
{
  _quadraticContribution = quadContribution;
}

Vector3 PointLight::GetPosition()
{
  return _position;
}

Vector3 PointLight::GetDiffuseColour()
{
  return _diffuseColour;
}

Vector3 PointLight::GetSpecularColour()
{
  return _specularColour;
}

float32 PointLight::GetConstContrib()
{
  return _constContribution;
}

float32 PointLight::GetLinearContrib()
{
  return _linearContribution;
}

float32 PointLight::GetQuadraticContrib()
{
  return _quadraticContribution;
}
}
