#include "DirDepthPassShader.hpp"

#include "../Rendering/ShaderCollection.h"

static const byte* LightTransformUniformName = "u_lightSpaceTransform";
static const byte* ModelTransformUniformName = "u_modelTransform";

DirDepthPassShader::DirDepthPassShader() :
  _modelTransform(Matrix4::Identity),
  _lightTransform(Matrix4::Identity),
  Shader(Rendering::ShaderCollection::ShaderDirectory + "DirectionalLightDepth.shader")
{}

DirDepthPassShader::~DirDepthPassShader()
{}

void DirDepthPassShader::SetLightSpaceTransform(const Matrix4& lightTransform)
{
  _lightTransform = lightTransform;
}

void DirDepthPassShader::SetModelSpaceTransform(const Matrix4& modelTransform)
{
  _modelTransform = modelTransform;
}

void DirDepthPassShader::Apply()
{
  Bind();
  SetMat4(ModelTransformUniformName, _modelTransform);
  SetMat4(LightTransformUniformName, _lightTransform);
}