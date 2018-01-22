#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"

namespace Rendering
{
class Texture;

class Material
{
public:
  Material();

  void SetAmbientColour(const Colour& colour);
  void SetDiffuseColour(const Colour& colour);
  void SetSpecularColour(const Colour& colour);
  void SetSpecularShininess(float32 shininess);
  void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
  void SetShadowsCast(bool castShadows) { _castShadows = castShadows; }

  bool HasDiffuseColour() const;
  bool HasSpecularColour() const;
  bool HasSpecularShininess() const;
  bool HasTexture(const std::string& name);

  inline const Colour& GetAmbientColour() const { return *_ambientColour; }
  inline const Colour& GetDiffuseColour() const { return *_diffuseColour; }
  inline const Colour& GetSpecularColour() const { return *_specularColour; }
  inline float32 GetSpecularExponent() const { return *_specularShininess; }

  std::shared_ptr<Texture> GetTexture(const std::string& name);
  bool AreShadowsCast() const { return _castShadows; }

  Material(Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(Material&) = delete;
  Material& operator=(Material&&) = delete;

private:
  std::unique_ptr<Colour> _ambientColour;
  std::unique_ptr<Colour> _diffuseColour;
  std::unique_ptr<Colour> _specularColour;
  std::unique_ptr<float32> _specularShininess;
  std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;

  bool _castShadows;
};
}