#pragma once
#include <memory>

#include "../Rendering/Shader.h"

class SkyBox;

namespace Rendering 
{
class ConstantBuffer;
}

class SkyBoxShader : public Rendering::Shader
{
public:
  SkyBoxShader();

  void SetTransformBuffer(std::weak_ptr<Rendering::ConstantBuffer> transformsBuffer);
  void SetSkyBox(std::weak_ptr<SkyBox> skyBox);

  void Apply();

private:
  std::weak_ptr<Rendering::ConstantBuffer> _transformsBuffer;
  std::weak_ptr<SkyBox> _skyBox;
};