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

  void SetDiffuseColour(const Vector3& colour);
  void SetSpecularColour(const Vector3& colour);
  void SetSpecularShininess(float32 shininess);
  void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
  void SetShadowsCast(bool castShadows) { _castShadows = castShadows; }

  bool HasDiffuseColour() const;
  bool HasSpecularColour() const;
  bool HasSpecularShininess() const;
  bool HasTexture(const std::string& name);

  std::shared_ptr<Texture> GetTexture(const std::string& name);
  bool AreShadowsCast() const { return _castShadows; }

private:
  Material(Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(Material&) = delete;
  Material& operator=(Material&&) = delete;

  std::unique_ptr<Vector3> _diffuseColour;
  std::unique_ptr<Vector3> _specularColour;
  std::unique_ptr<float32> _specularShininess;
  std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;

  bool _castShadows;
};
}