#pragma once
#include <memory>
#include <string>

#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Rendering/Shader.h"

namespace Rendering {
  class Material;
  class Texture;
}

class GeometryPassShader : public Rendering::Shader
{
public:
  GeometryPassShader();
  ~GeometryPassShader();
  
  void SetModelTransform(const Matrix4& modelTransform);
  void SetMaterialProperties(std::shared_ptr<Rendering::Material> material);  
  void SetTransformsBindingPoint(uint32 bindingPoint);
  void SetViewDirection(const Vector3& viewDirection);

  void Apply();

private:
  void SetDiffuseColour(const Colour& colour);
  void SetDiffuseMap(std::weak_ptr<Rendering::Texture> diffuseMap);
  void SetNormalMap(std::weak_ptr<Rendering::Texture> normalMap);
  void SetSpecularMap(std::weak_ptr<Rendering::Texture> specularMap);
  void SetDepthMap(std::weak_ptr<Rendering::Texture> depthMap);

private:
  std::weak_ptr<Rendering::Texture> _diffuseMap;
  std::weak_ptr<Rendering::Texture> _normalMap;
  std::weak_ptr<Rendering::Texture> _specularMap;
  std::weak_ptr<Rendering::Texture> _depthMap;
  Colour _diffuseColour;
  Matrix4 _modelTransform;
  Vector3 _viewDirection;
  uint32 _transformsBufferBindingIndex;
};