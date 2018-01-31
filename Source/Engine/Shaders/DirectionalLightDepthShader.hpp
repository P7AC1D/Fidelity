#pragma once
#include <string>

#include "../Maths/Matrix4.hpp"
#include "../Rendering/Shader.h"

class DirectionalLightDepthShader : public Rendering::Shader
{
public:
  DirectionalLightDepthShader(const std::string& shaderDirectory);
  ~DirectionalLightDepthShader();

  void SetLightSpaceTransform(const Matrix4& lightTransform);
  void SetModelSpaceTransform(const Matrix4& modelTransform);

  void Apply();

private:
  Matrix4 _modelTransform;
  Matrix4 _lightTransform;
};