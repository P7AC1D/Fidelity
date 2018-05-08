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

  inline void SetAmbientColour(const Colour& colour) { _ambientColour = colour; }
  inline void SetDiffuseColour(const Colour& colour) { _diffuseColour = colour; }
  inline void SetSpecularColour(const Colour& colour) { _specularColour = colour; }
  inline void SetSpecularShininess(float32 shininess) { _specularShininess = shininess; }
  void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
  void SetShadowsCast(bool castShadows) { _castShadows = castShadows; }

  bool HasTexture(const std::string& name)  const;

  inline const Colour& GetAmbientColour() const { return _ambientColour; }
  inline const Colour& GetDiffuseColour() const { return _diffuseColour; }
  inline const Colour& GetSpecularColour() const { return _specularColour; }
  inline float32 GetSpecularExponent() const { return _specularShininess; }

  std::shared_ptr<Texture> GetTexture(const std::string& name) const;
  bool AreShadowsCast() const { return _castShadows; }

private:
  std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;
  bool _castShadows;
  Colour _ambientColour;
  Colour _diffuseColour;
  Colour _specularColour;
  float32 _specularShininess;
};
}