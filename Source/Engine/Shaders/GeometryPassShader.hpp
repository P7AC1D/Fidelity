#pragma once
#include <memory>
#include <string>

#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Rendering/Shader.h"

namespace Rendering {
  class ConstantBuffer;
  class Material;
  class Texture;
}

class GeometryPassShader : public Rendering::Shader
{
public:
  GeometryPassShader();
  ~GeometryPassShader();
  
  void SetModelTransform(const Matrix4& modelTransform);
  void SetMaterialProperties(const Rendering::Material& material);  
  void SetTransformsUniformbuffer(std::weak_ptr<Rendering::ConstantBuffer> transformsBuffer);

  void Apply();

private:
  void SetDiffuseColour(const Colour& colour);
  void SetDiffuseMap(std::weak_ptr<Rendering::Texture> diffuseMap);
  void SetNormalMap(std::weak_ptr<Rendering::Texture> normalMap);
  void SetSpecularMap(std::weak_ptr<Rendering::Texture> specularMap);

private:
  std::weak_ptr<Rendering::Texture> _diffuseMap;
  std::weak_ptr<Rendering::Texture> _normalMap;
  std::weak_ptr<Rendering::Texture> _specularMap;
  std::weak_ptr<Rendering::ConstantBuffer> _transformsBuffer;  
  Colour _diffuseColour;
  Matrix4 _modelTransform;
};