#include "Material.h"

#include "../RenderApi/Texture.hpp"

Material::Material() : _id(0), _specularExponent(1.0f)
{
}

Material &Material::setAmbientColour(const Colour &ambientColour)
{
  _ambientColour = ambientColour;
  return *this;
}

Material &Material::setDiffuseColour(const Colour &diffuseColour)
{
  _diffuseColour = diffuseColour;
  return *this;
}

Material &Material::setSpecularColour(const Colour &specularColour)
{
  _specularColour = specularColour;
  return *this;
}

Material &Material::setSpecularExponent(float32 specularExponent)
{
  _specularExponent = specularExponent;
  return *this;
}

Material &Material::setDiffuseTexture(const std::shared_ptr<Texture> &diffuseTexture)
{
  _diffuseTexture = diffuseTexture;
  return *this;
}

Material &Material::setNormalTexture(const std::shared_ptr<Texture> &normalTexture)
{
  _normalTexture = normalTexture;
  return *this;
}

Material &Material::setSpecularTexture(const std::shared_ptr<Texture> &specularTexture)
{
  _specularTexture = specularTexture;
  return *this;
}