#include "LightingPassShader.hpp"

#include <cassert>

#include "../Rendering/Texture.h"

static const byte* ViewDirUniformName = "u_viewDir";
static const byte* LightDirUniformName = "u_lightDir";
static const byte* LightColourUniformName = "u_lightColour";
static const byte* GPositionSamplerName = "u_gPosition";
static const byte* GNormalSamplerName = "u_gNormal";
static const byte* GAlbedoSpecSamplerName = "u_gAlbedoSpec";
static const uint32 GPositionTextureSlot = 0;
static const uint32 GNormalTextureSlot = 1;
static const uint32 GAlbedoSpecTextureSlot = 2;

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

void LightingPassShader::GeometryBuffer(std::shared_ptr<Rendering::FrameBuffer> gBuffer)
{
  _gPosition = gBuffer->GetColourTexture0();
  _gNormal = gBuffer->GetColourTexture1();
  _gAlbedoSpec = gBuffer->GetColourTexture2();
}

void LightingPassShader::Apply()
{
  SetVec3(ViewDirUniformName, _dirLightDir);
  SetVec3(LightDirUniformName, _dirLightDir);
  SetVec3(LightColourUniformName, _dirLightCol.ToVec3());
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
}