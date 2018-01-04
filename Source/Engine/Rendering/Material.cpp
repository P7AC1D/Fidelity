#include "Material.h"

namespace Rendering
{
Material::Material() :
  _castShadows(true)
{}

void Material::SetDiffuseColour(const Vector3& colour)
{  
  _diffuseColour.reset(new Vector3(colour[0], colour[1], colour[2]));
}

void Material::SetSpecularColour(const Vector3& colour)
{
  _specularColour.reset(new Vector3(colour[0], colour[1], colour[2]));
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