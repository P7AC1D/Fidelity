#pragma once
#include <memory>
#include <string>

#include "../SceneManagement/Light.h"
#include "../Maths/Colour.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/FrameBuffer.h"
#include "../Rendering/Shader.h"

class LightingPassShader : public Rendering::Shader
{
public:
  LightingPassShader(const std::string& shaderDirectory);
  ~LightingPassShader();

  void SetViewDirection(const Vector3& viewDirection);
  void SetDirectionalLight(const Light& directionalLight);
  void GeometryBuffer(std::shared_ptr<Rendering::FrameBuffer> gBuffer);

  void Apply();

private:
  Vector3 _viewDir;
  Vector3 _dirLightDir;
  Colour _dirLightCol;
  std::weak_ptr<Rendering::Texture> _gPosition;
  std::weak_ptr<Rendering::Texture> _gNormal;
  std::weak_ptr<Rendering::Texture> _gAlbedoSpec;
};