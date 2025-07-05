#include "Material.h"

#include "../RenderApi/Texture.hpp"

static uint32 ID_COUNTER = 0;

Material::Material() : _id(ID_COUNTER++),
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
  invalidateSortingKeys();
  return *this;
}

Material &Material::setNormalTexture(const std::shared_ptr<Texture> &normalTexture)
{
  _normalTexture = normalTexture;
  _normalEnabled = true;
  invalidateSortingKeys();
  return *this;
}

Material &Material::setMetallicTexture(const std::shared_ptr<Texture> &metallicTexture)
{
  _metallicTexture = metallicTexture;
  _metallicEnabled = true;
  invalidateSortingKeys();
  return *this;
}

Material &Material::setRoughnessTexture(const std::shared_ptr<Texture> &roughnessTexture)
{
  _roughnessTexture = roughnessTexture;
  _roughnessEnabled = true;
  invalidateSortingKeys();
  return *this;
}

Material &Material::setOcclusionTexture(const std::shared_ptr<Texture> &occlusionTexture)
{
  _occlusionTexture = occlusionTexture;
  _occlusionEnabled = true;
  invalidateSortingKeys();
  return *this;
}

Material &Material::setOpacityTexture(const std::shared_ptr<Texture> &opacityTexture)
{
  _opacityTexture = opacityTexture;
  _opacityEnabled = true;
  invalidateSortingKeys();
  return *this;
}

uint32 Material::getShaderID() const
{
  if (_sortingKeysDirty) {
    updateSortingKeys();
  }
  return _cachedShaderID;
}

uint64 Material::getTextureHash() const
{
  if (_sortingKeysDirty) {
    updateSortingKeys();
  }
  return _cachedTextureHash;
}

void Material::updateSortingKeys() const
{
  // Generate shader ID based on enabled texture flags
  // This represents different shader variants/paths
  _cachedShaderID = 0;
  if (_diffuseEnabled) _cachedShaderID |= (1 << 0);
  if (_normalEnabled) _cachedShaderID |= (1 << 1);
  if (_metallicEnabled) _cachedShaderID |= (1 << 2);
  if (_roughnessEnabled) _cachedShaderID |= (1 << 3);
  if (_occlusionEnabled) _cachedShaderID |= (1 << 4);
  if (_opacityEnabled) _cachedShaderID |= (1 << 5);
  
  // Generate texture hash based on texture pointers
  // This groups materials with similar texture binding requirements
  _cachedTextureHash = 0;
  
  // Simple hash combination using FNV-1a algorithm
  const uint64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
  const uint64 FNV_PRIME = 1099511628211ULL;
  
  _cachedTextureHash = FNV_OFFSET_BASIS;
  
  // Hash each texture pointer if enabled
  if (_diffuseEnabled && _diffuseTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_diffuseTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  if (_normalEnabled && _normalTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_normalTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  if (_metallicEnabled && _metallicTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_metallicTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  if (_roughnessEnabled && _roughnessTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_roughnessTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  if (_occlusionEnabled && _occlusionTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_occlusionTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  if (_opacityEnabled && _opacityTexture) {
    uint64 ptr = reinterpret_cast<uint64>(_opacityTexture.get());
    _cachedTextureHash ^= ptr;
    _cachedTextureHash *= FNV_PRIME;
  }
  
  _sortingKeysDirty = false;
}

void Material::invalidateSortingKeys()
{
  _sortingKeysDirty = true;
}
