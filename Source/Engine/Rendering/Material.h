#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"

namespace Rendering
{
class Texture;

class Material
{
public:
  Material();

  inline void SetAmbientColour(const Vector3& colour) { _ambientColour.reset(new Vector3(colour[0], colour[1], colour[2])); }
  void SetDiffuseColour(const Vector3& colour);
  void SetSpecularColour(const Vector3& colour);
  void SetSpecularShininess(float32 shininess);
  void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
  void SetShadowsCast(bool castShadows) { _castShadows = castShadows; }

  bool HasDiffuseColour() const;
  bool HasSpecularColour() const;
  bool HasSpecularShininess() const;
  bool HasTexture(const std::string& name);

  inline const Vector3& GetAmbientColour() const { return *_ambientColour; }
  inline const Vector3& GetDiffuseColour() const { return *_diffuseColour; }
  inline const Vector3& GetSpecularColour() const { return *_specularColour; }
  inline float32 GetSpecularExponent() const { return *_specularShininess; }

  std::shared_ptr<Texture> GetTexture(const std::string& name);
  bool AreShadowsCast() const { return _castShadows; }

  Material(Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(Material&) = delete;
  Material& operator=(Material&&) = delete;

private:
  std::unique_ptr<Vector3> _ambientColour;
  std::unique_ptr<Vector3> _diffuseColour;
  std::unique_ptr<Vector3> _specularColour;
  std::unique_ptr<float32> _specularShininess;
  std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;

  bool _castShadows;
};
}