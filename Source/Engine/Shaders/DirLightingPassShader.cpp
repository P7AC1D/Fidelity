#include "DirLightingPassShader.hpp"

#include <cassert>

#include "../Rendering/RenderTarget.hpp"
#include "../Rendering/ShaderCollection.h"
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

DirLightingPassShader::DirLightingPassShader() :
  _viewDir(Vector3::Identity),
  _dirLightDir(Vector3::Identity),
  _dirLightCol(Colour::White),
  Shader(Rendering::ShaderCollection::ShaderDirectory + "DirLighting.shader")
{}

DirLightingPassShader::~DirLightingPassShader()
{}

void DirLightingPassShader::SetViewDirection(const Vector3& viewDirection)
{
  _viewDir = viewDirection;
}

void DirLightingPassShader::SetDirectionalLight(const Light& directionalLight)
{
  assert(directionalLight.GetType() == LightType::Directional);

  _dirLightDir = Vector3::Normalize(directionalLight.GetDirection());
  _dirLightCol = directionalLight.GetColour();
}

void DirLightingPassShader::SetGeometryBuffer(std::shared_ptr<Rendering::RenderTarget> gBuffer)
{
  _gPosition = gBuffer->GetColourBuffer(0);
  _gNormal = gBuffer->GetColourBuffer(1);
  _gAlbedoSpec = gBuffer->GetColourBuffer(2);
}

void DirLightingPassShader::SetDirLightDepthBuffer(std::shared_ptr<Rendering::RenderTarget> depthBuffer)
{
  _dirLightDepth = depthBuffer->GetDepthStencilBuffer();
  _shadowTexelSize = Vector2((1.0f / depthBuffer->GetDesc().Width), (1.0f / depthBuffer->GetDesc().Height));
}

void DirLightingPassShader::SetLightSpaceTransform(const Matrix4& transform)
{
  _lightSpace = transform;
}

void DirLightingPassShader::Apply()
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