#pragma once
#include <memory>

#include "../Rendering/Shader.h"

class SkyBox;

class SkyBoxShader : public Rendering::Shader
{
public:
  SkyBoxShader();

  void SetTransformsBindingPoint(uint32 bindingPoint);
  void SetSkyBox(std::weak_ptr<SkyBox> skyBox);

  void Apply();

private:
  std::weak_ptr<SkyBox> _skyBox;
  uint32 _transformsBufferBindingIndex;
};