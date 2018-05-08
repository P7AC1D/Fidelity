#pragma once
#include <string>

#include "../Maths/Matrix4.hpp"
#include "../Rendering/Shader.h"

class DirDepthPassShader : public Rendering::Shader
{
public:
  DirDepthPassShader();
  ~DirDepthPassShader();

  void SetLightSpaceTransform(const Matrix4& lightTransform);
  void SetModelSpaceTransform(const Matrix4& modelTransform);

  void Apply();

private:
  Matrix4 _modelTransform;
  Matrix4 _lightTransform;
};