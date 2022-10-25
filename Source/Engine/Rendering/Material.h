#pragma once
#include <memory>
#include "../Core/Maths.h"

class Texture;

class Material
{
public:
  Material();

  Material &setDiffuseColour(const Colour &diffuseColour);
  Material &setMetalness(float32 metalness);
  Material &setRoughness(float32 roughness);
  Material &setOpacity(float32 opacity);

  Material &setDiffuseTexture(const std::shared_ptr<Texture> &diffuseTexture);
  Material &setNormalTexture(const std::shared_ptr<Texture> &normalTexture);
  Material &setMetallicTexture(const std::shared_ptr<Texture> &metallicTexture);
  Material &setRoughnessTexture(const std::shared_ptr<Texture> &roughnessTexture);
  Material &setOcclusionTexture(const std::shared_ptr<Texture> &occlusionTexture);
  Material &setOpacityTexture(const std::shared_ptr<Texture> &opacityTexture);

  uint32 getId() const { return _id; }
  Colour getDiffuseColour() const { return _diffuseColour; }
  float32 getMetalness() const { return _metalness; }
  float32 getRoughness() const { return _roughness; }

  std::shared_ptr<Texture> getDiffuseTexture() const { return _diffuseTexture; }
  std::shared_ptr<Texture> getNormalTexture() const { return _normalTexture; }
  std::shared_ptr<Texture> getMetallicTexture() const { return _metallicTexture; }
  std::shared_ptr<Texture> getRoughnessTexture() const { return _roughnessTexture; }
  std::shared_ptr<Texture> getOcclusionTexture() const { return _occlusionTexture; }
  std::shared_ptr<Texture> getOpacityTexture() const { return _opacityTexture; }

  bool hasDiffuseTexture() const { return _diffuseTexture != nullptr; }
  bool hasNormalTexture() const { return _normalTexture != nullptr; }
  bool hasMetallicTexture() const { return _metallicTexture != nullptr; }
  bool hasRoughnessTexture() const { return _roughnessTexture != nullptr; }
  bool hasOcclusionTexture() const { return _occlusionTexture != nullptr; }
  bool hasOpacityTexture() const { return _opacityTexture != nullptr; }

  void enableDiffuseTexture(bool enable) { _diffuseEnabled = enable; }
  void enableNormalTexture(bool enable) { _normalEnabled = enable; }
  void enableMetallicTexture(bool enable) { _metallicEnabled = enable; }
  void enableRoughnessTexture(bool enable) { _roughnessEnabled = enable; }
  void enableOcclusionTexture(bool enable) { _occlusionEnabled = enable; }
  void enableOppacityTexture(bool enable) { _opacityEnabled = enable; }

  bool diffuseTextureEnabled() const { return _diffuseEnabled; }
  bool normalTextureEnabled() const { return _normalEnabled; }
  bool metallicTextureEnabled() const { return _metallicEnabled; }
  bool roughnessTextureEnabled() const { return _roughnessEnabled; }
  bool occlusionTextureEnabled() const { return _occlusionEnabled; }
  bool opacityTextureEnabled() const { return _opacityEnabled; }

private:
  uint32 _id;
  Colour _diffuseColour;
  float32 _metalness;
  float32 _roughness;
  float32 _oppacity;

  bool _diffuseEnabled;
  bool _normalEnabled;
  bool _metallicEnabled;
  bool _roughnessEnabled;
  bool _occlusionEnabled;
  bool _opacityEnabled;

  std::shared_ptr<Texture> _diffuseTexture;
  std::shared_ptr<Texture> _normalTexture;
  std::shared_ptr<Texture> _metallicTexture;
  std::shared_ptr<Texture> _roughnessTexture;
  std::shared_ptr<Texture> _occlusionTexture;
  std::shared_ptr<Texture> _opacityTexture;
};