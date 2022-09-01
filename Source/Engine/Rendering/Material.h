#pragma once
#include <memory>
#include "../Core/Maths.h"

class Texture;

class Material
{
public:
  Material();

  Material &setAmbientColour(const Colour &ambientColour);
  Material &setDiffuseColour(const Colour &diffuseColour);
  Material &setSpecularColour(const Colour &specularColour);
  Material &setSpecularExponent(float32 specularExponent);

  Material &setDiffuseTexture(const std::shared_ptr<Texture> &diffuseTexture);
  Material &setNormalTexture(const std::shared_ptr<Texture> &normalTexture);
  Material &setSpecularTexture(const std::shared_ptr<Texture> &specularTexture);

  uint32 getId() const { return _id; }
  Colour getAmbientColour() const { return _ambientColour; }
  Colour getDiffuseColour() const { return _diffuseColour; }
  Colour getSpecularColour() const { return _specularColour; }
  float32 getSpecularExponent() const { return _specularExponent; }

  std::shared_ptr<Texture> getDiffuseTexture() const { return _diffuseTexture; }
  std::shared_ptr<Texture> getNormalTexture() const { return _normalTexture; }
  std::shared_ptr<Texture> getSpecularTexture() const { return _specularTexture; }

  bool hasDiffuseTexture() const { return _diffuseTexture != nullptr; }
  bool hasNormalTexture() const { return _normalTexture != nullptr; }
  bool hasSpecularTexture() const { return _specularTexture != nullptr; }

private:
  uint32 _id;
  Colour _ambientColour;
  Colour _diffuseColour;
  Colour _specularColour;
  float32 _specularExponent;

  std::shared_ptr<Texture> _diffuseTexture;
  std::shared_ptr<Texture> _normalTexture;
  std::shared_ptr<Texture> _specularTexture;
  std::shared_ptr<Texture> _opacityTexture;
  std::shared_ptr<Texture> _depthTexture;
};