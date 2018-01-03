#include "Material.hpp"

#include "RenderingSystem.hpp"

Material::Material()
{
  static uint32 idCount = 0;
  _id = idCount;
  idCount++;
  
  RenderingSystem::Instance()->RegisterMaterial(this);
}

Material::~Material()
{
  RenderingSystem::Instance()->DeregisterMaterial(_id);
}

void Material::SetDiffuseFactor(uint32 kd)
{
  _diffuseFactor = kd;
  _needsUpdate = true;
}

void Material::SetDiffuseMap(const std::shared_ptr<Texture> diffuseMap)
{
  _diffuseMap = diffuseMap;
  _needsUpdate = true;
}
