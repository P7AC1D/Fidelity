#include "GeometryPassShader.hpp"

#include "../Rendering/ConstantBuffer.h"
#include "../Rendering/Material.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/Texture.hpp"

using namespace Rendering;

static const byte* DiffuseMappingEnabled = "u_diffuseMappingEnabled";
static const byte* NormalMappingEnabled = "u_normalMappingEnabled";
static const byte* SpecularMappingEnabled = "u_specularMappingEnabled";
static const byte* DepthMappingEnabled = "u_depthMappingEnabled";
static const byte* ViewDirUniformName = "u_viewDir";
static const byte* DiffuseColourUniformName = "u_material.DiffuseColour";
static const byte* DiffuseMapSamplerUniformName = "u_material.DiffuseMap";
static const byte* NormalMapSamplerUniformName = "u_material.NormalMap";
static const byte* SpecularMapSamplerUniformName = "u_material.SpecularMap";
static const byte* DepthMapSamplerUniformName = "u_material.DepthMap";
static const byte* ModelTransformUniformName = "u_model";
static const byte* TransformsUniformBufferName = "Transforms";
static const uint32 DiffuseMapTextureSlot = 0;
static const uint32 NormalMapTextureSlot = 1;
static const uint32 SpecularMapTextureSlot = 2;
static const uint32 DepthMapTextureSlot = 3;
static const uint32 TransformsUniformBufferBindingPoint = 0;

GeometryPassShader::GeometryPassShader():
  Shader(Rendering::ShaderCollection::ShaderDirectory + "GBuffer.shader"),
  _diffuseColour(Colour::White),
  _modelTransform(Matrix4::Identity)
{}

GeometryPassShader::~GeometryPassShader()
{}

void GeometryPassShader::SetModelTransform(const Matrix4& modelTransform)
{
  if (_modelTransform == modelTransform)
  {
    return;
  }  
  _modelTransform = modelTransform;
}

void GeometryPassShader::SetMaterialProperties(std::shared_ptr<Rendering::Material> material)
{
  SetDiffuseColour(material->GetDiffuseColour());
  if (material->HasTexture("DiffuseMap"))
  {
    SetDiffuseMap(material->GetTexture("DiffuseMap"));
  }
  else
  {
    _diffuseMap.reset();
  }

  if (material->HasTexture("NormalMap"))
  {
    SetNormalMap(material->GetTexture("NormalMap"));
  }
  else
  {
    _normalMap.reset();
  }

  if (material->HasTexture("SpecularMap"))
  {
    SetSpecularMap(material->GetTexture("SpecularMap"));
  }
  else
  {
    _specularMap.reset();
  }

  if (material->HasTexture("DepthMap"))
  {
    SetDepthMap(material->GetTexture("DepthMap"));
  }
  else
  {
    _depthMap.reset();
  }
}

void GeometryPassShader::SetTransformsUniformbuffer(std::weak_ptr<ConstantBuffer> transformsBuffer)
{
  _transformsBuffer = transformsBuffer;
}

void GeometryPassShader::SetViewDirection(const Vector3& viewDirection)
{
  _viewDirection = viewDirection;
}

void GeometryPassShader::Apply()
{
  Bind();
  SetVec3(DiffuseColourUniformName, _diffuseColour.ToVec3());
  SetMat4(ModelTransformUniformName, _modelTransform);
  SetVec3(ViewDirUniformName, _viewDirection);

  if (!_diffuseMap.expired())
  {
    _diffuseMap.lock()->BindToTextureSlot(DiffuseMapTextureSlot);
    SetInt(DiffuseMapSamplerUniformName, DiffuseMapTextureSlot);
    SetBool(DiffuseMappingEnabled, true);
  }
  else
  {
    SetBool(DiffuseMappingEnabled, false);
  }

  if (!_normalMap.expired())
  {
    _normalMap.lock()->BindToTextureSlot(NormalMapTextureSlot);
    SetInt(NormalMapSamplerUniformName, NormalMapTextureSlot);
    SetBool(NormalMappingEnabled, true);
  }
  else
  {
    SetBool(NormalMappingEnabled, false);
  }

  if (!_specularMap.expired())
  {
    _specularMap.lock()->BindToTextureSlot(SpecularMapTextureSlot);
    SetInt(SpecularMapSamplerUniformName, SpecularMapTextureSlot);
    SetBool(SpecularMappingEnabled, true);
  }
  else
  {
    SetBool(SpecularMappingEnabled, false);
  }

  if (!_depthMap.expired())
  {
    _depthMap.lock()->BindToTextureSlot(DepthMapTextureSlot);
    SetInt(DepthMapSamplerUniformName, DepthMapTextureSlot);
    SetBool(DepthMappingEnabled, true);
  }
  else
  {
    SetBool(DepthMappingEnabled, false);
  }

  if (!_transformsBuffer.expired())
  {
    BindUniformBlock(GetUniformBlockBindingPoint(TransformsUniformBufferName), TransformsUniformBufferBindingPoint, _transformsBuffer.lock()->GetId());
  }
}

void GeometryPassShader::SetDiffuseColour(const Colour& colour)
{
  if (colour == _diffuseColour)
  {
    return;
  }
  _diffuseColour = colour;
}

void GeometryPassShader::SetDiffuseMap(std::weak_ptr<Texture> diffuseMap)
{
  _diffuseMap = diffuseMap;
}

void GeometryPassShader::SetNormalMap(std::weak_ptr<Rendering::Texture> normalMap)
{
  _normalMap = normalMap;
}

void GeometryPassShader::SetSpecularMap(std::weak_ptr<Rendering::Texture> specularMap)
{
  _specularMap = specularMap;
}

void GeometryPassShader::SetDepthMap(std::weak_ptr<Rendering::Texture> depthMap)
{
  _depthMap = depthMap;
}
