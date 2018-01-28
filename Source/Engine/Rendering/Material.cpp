#include "Material.h"

namespace Rendering
{
Material::Material() :
_castShadows(true),
_specularColour(Colour::Black),
_specularShininess(0.0)
{}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture)
{
  _textures[name] = texture;
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