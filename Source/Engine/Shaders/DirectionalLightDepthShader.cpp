#include "DirectionalLightDepthShader.hpp"

static const byte* LightTransformUniformName = "u_lightSpaceTransform";
static const byte* ModelTransformUniformName = "u_modelTransform";

DirectionalLightDepthShader::DirectionalLightDepthShader(const std::string& shaderDirectory) :
  _modelTransform(Matrix4::Identity),
  _lightTransform(Matrix4::Identity),
  Shader(shaderDirectory + "DirectionalLightDepth.shader")
{}

DirectionalLightDepthShader::~DirectionalLightDepthShader()
{}

void DirectionalLightDepthShader::SetLightSpaceTransform(const Matrix4& lightTransform)
{
  _lightTransform = lightTransform;
}

void DirectionalLightDepthShader::SetModelSpaceTransform(const Matrix4& modelTransform)
{
  _modelTransform = modelTransform;
}

void DirectionalLightDepthShader::Apply()
{
  Bind();
  SetMat4(ModelTransformUniformName, _modelTransform);
  SetMat4(LightTransformUniformName, _lightTransform);
}