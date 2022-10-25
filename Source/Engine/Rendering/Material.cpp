#include "Material.h"

#include "../RenderApi/Texture.hpp"

Material::Material() : _id(0),
                       _diffuseEnabled(false),
                       _normalEnabled(false),
                       _metallicEnabled(false),
                       _roughnessEnabled(false),
                       _occlusionEnabled(false),
                       _opacityEnabled(false),
                       _metalness(0.0f),
                       _roughness(0.0f)
{
}

Material &Material::setDiffuseColour(const Colour &diffuseColour)
{
  _diffuseColour = diffuseColour;
  return *this;
}

Material &Material::setMetalness(float32 metalness)
{
  _metalness = metalness;
  return *this;
}

Material &Material::setRoughness(float32 roughness)
{
  _roughness = roughness;
  return *this;
}

Material &Material::setOpacity(float32 opacity)
{
  _oppacity = opacity;
  return *this;
}

Material &Material::setDiffuseTexture(const std::shared_ptr<Texture> &diffuseTexture)
{
  _diffuseTexture = diffuseTexture;
  _diffuseEnabled = true;
  return *this;
}

Material &Material::setNormalTexture(const std::shared_ptr<Texture> &normalTexture)
{
  _normalTexture = normalTexture;
  _normalEnabled = true;
  return *this;
}

Material &Material::setMetallicTexture(const std::shared_ptr<Texture> &metallicTexture)
{
  _metallicTexture = metallicTexture;
  _metallicEnabled = true;
  return *this;
}

Material &Material::setRoughnessTexture(const std::shared_ptr<Texture> &roughnessTexture)
{
  _roughnessTexture = roughnessTexture;
  _roughnessEnabled = true;
  return *this;
}

Material &Material::setOcclusionTexture(const std::shared_ptr<Texture> &occlusionTexture)
{
  _occlusionTexture = occlusionTexture;
  _occlusionEnabled = true;
  return *this;
}

Material &Material::setOpacityTexture(const std::shared_ptr<Texture> &opacityTexture)
{
  _opacityTexture = opacityTexture;
  _opacityEnabled = true;
  return *this;
}