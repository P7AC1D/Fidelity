#include "Material.h"

#include "../RenderApi/Texture.hpp"

Material::Material() :
  _castShadows(true),
  _ambientColour(Colour::White),
  _diffuseColour(Colour::White),
  _specularColour(Colour::White),
  _specularShininess(1.0f)
{}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture)
{
  _textures[name] = texture;
}

bool Material::HasTexture(const std::string& name) const
{
  auto iter = _textures.find(name);
  return iter != _textures.end();
}

std::shared_ptr<Texture> Material::GetTexture(const std::string& name) const
{
  auto iter = _textures.find(name);
  if (iter == _textures.end())
  {
    return nullptr;
  }
  return iter->second;
}