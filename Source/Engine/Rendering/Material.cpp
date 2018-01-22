#include "Material.h"

namespace Rendering
{
Material::Material() :
_castShadows(true)
{}

void Material::SetAmbientColour(const Colour& colour)
{
  _ambientColour.reset(new Colour(colour));
}

void Material::SetDiffuseColour(const Colour& colour)
{  
  _diffuseColour.reset(new Colour(colour));
}

void Material::SetSpecularColour(const Colour& colour)
{
  _specularColour.reset(new Colour(colour));
}

void Material::SetSpecularShininess(float32 shininess)
{
  _specularShininess.reset(new float32(shininess));
}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture)
{
  _textures[name] = texture;
}

bool Material::HasDiffuseColour() const
{
  return _diffuseColour != nullptr;
}

bool Material::HasSpecularColour() const
{
  return _specularColour != nullptr;
}

bool Material::HasSpecularShininess() const
{
  return _specularShininess != nullptr;
}

bool Material::HasTexture(const std::string& name)
{
  auto iter = _textures.find(name);
  return iter != _textures.end();
}

std::shared_ptr<Texture> Material::GetTexture(const std::string& name)
{
  auto iter = _textures.find(name);
  if (iter == _textures.end())
  {
    return nullptr;
  }
  return iter->second;
}
}