#include "LightingPassShader.hpp"

#include <cassert>

#include "../Rendering/Texture.h"

static const byte* ShadowTexelSizeUniformName = "u_shadowTexelSize";
static const byte* ViewDirUniformName = "u_viewDir";
static const byte* LightDirUniformName = "u_lightDir";
static const byte* LightColourUniformName = "u_lightColour";
static const byte* LightTransformUniformName = "u_lightTransform";
static const byte* GPositionSamplerName = "u_gPosition";
static const byte* GNormalSamplerName = "u_gNormal";
static const byte* GAlbedoSpecSamplerName = "u_gAlbedoSpec";
static const byte* DirLightDepthSamplerName = "u_dirLightDepth";
static const uint32 GPositionTextureSlot = 0;
static const uint32 GNormalTextureSlot = 1;
static const uint32 GAlbedoSpecTextureSlot = 2;
static const uint32 DirLightDepthTextureSlot = 3;

LightingPassShader::LightingPassShader(const std::string& shaderDirectory) :
  _viewDir(Vector3::Identity),
  _dirLightDir(Vector3::Identity),
  _dirLightCol(Colour::White),
  Shader(shaderDirectory + "DeferredLightingPass.shader")
{}

LightingPassShader::~LightingPassShader()
{}

void LightingPassShader::SetViewDirection(const Vector3& viewDirection)
{
  _viewDir = viewDirection;
}

void LightingPassShader::SetDirectionalLight(const Light& directionalLight)
{
  assert(directionalLight.GetType() == LightType::Directional);

  _dirLightDir = Vector3::Normalize(directionalLight.GetDirection());
  _dirLightCol = directionalLight.GetColour();
}

void LightingPassShader::SetGeometryBuffer(std::shared_ptr<Rendering::FrameBuffer> gBuffer)
{
  _gPosition = gBuffer->GetColourTexture0();
  _gNormal = gBuffer->GetColourTexture1();
  _gAlbedoSpec = gBuffer->GetColourTexture2();
}

void LightingPassShader::SetDirLightDepthBuffer(std::shared_ptr<Rendering::FrameBuffer> depthBuffer)
{
  _dirLightDepth = depthBuffer->GetDepthTexture();
  _shadowTexelSize = Vector2(1.0f / depthBuffer->GetWidth(), 1.0f / depthBuffer->GetHeight());
}

void LightingPassShader::SetLightSpaceTransform(const Matrix4& transform)
{
  _lightSpace = transform;
}

void LightingPassShader::Apply()
{
  SetVec2(ShadowTexelSizeUniformName, _shadowTexelSize);
  SetVec3(ViewDirUniformName, _dirLightDir);
  SetVec3(LightDirUniformName, _dirLightDir);
  SetVec3(LightColourUniformName, _dirLightCol.ToVec3());
  SetMat4(LightTransformUniformName, _lightSpace);
  if (!_gPosition.expired())
  {
    _gPosition.lock()->BindToTextureSlot(GPositionTextureSlot);
    SetInt(GPositionSamplerName, GPositionTextureSlot);
  }
  if (!_gNormal.expired())
  {
    _gNormal.lock()->BindToTextureSlot(GNormalTextureSlot);
    SetInt(GNormalSamplerName, GNormalTextureSlot);
  }
  if (!_gAlbedoSpec.expired())
  {
    _gAlbedoSpec.lock()->BindToTextureSlot(GAlbedoSpecTextureSlot);
    SetInt(GAlbedoSpecSamplerName, GAlbedoSpecTextureSlot);
  }

  if (!_dirLightDepth.expired())
  {
    _dirLightDepth.lock()->BindToTextureSlot(DirLightDepthTextureSlot);
    SetInt(DirLightDepthSamplerName, DirLightDepthTextureSlot);
  }
}