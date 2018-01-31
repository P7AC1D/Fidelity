#pragma once
#include <memory>
#include <string>

#include "../SceneManagement/Light.h"
#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Vector2.hpp"
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
  void SetGeometryBuffer(std::shared_ptr<Rendering::FrameBuffer> gBuffer);

  void SetDirLightDepthBuffer(std::shared_ptr<Rendering::FrameBuffer> depthBuffer);
  void SetLightSpaceTransform(const Matrix4& transform);

  void Apply();

private:
  std::weak_ptr<Rendering::Texture> _gPosition;
  std::weak_ptr<Rendering::Texture> _gNormal;
  std::weak_ptr<Rendering::Texture> _gAlbedoSpec;
  std::weak_ptr<Rendering::Texture> _dirLightDepth;
  Colour _dirLightCol;
  Vector2 _shadowTexelSize;
  Vector3 _viewDir;
  Vector3 _dirLightDir;
  Matrix4 _lightSpace;
};