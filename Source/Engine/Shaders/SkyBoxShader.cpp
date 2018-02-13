#include "SkyBoxShader.hpp"

#include "../Rendering/ConstantBuffer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/TextureCube.hpp"
#include "../SceneManagement/SkyBox.hpp"

static const byte* TransformsUniformBufferName = "Transforms";
static const byte* SkyBoxUniformName = "u_skyBox";
static const uint32 TransformsUniformBufferBindingPoint = 0;
static const uint32 SkyBoxTextureSlot = 0;

SkyBoxShader::SkyBoxShader():
  Shader(Rendering::ShaderCollection::ShaderDirectory + "SkyBox.shader")
{
}

void SkyBoxShader::SetTransformBuffer(std::weak_ptr<Rendering::ConstantBuffer> transformsBuffer)
{
  _transformsBuffer = transformsBuffer;
}

void SkyBoxShader::SetSkyBox(std::weak_ptr<SkyBox> skyBox)
{
  _skyBox = skyBox;
}

void SkyBoxShader::Apply()
{
  Bind();
  if (!_transformsBuffer.expired())
  {
    BindUniformBlock(GetUniformBlockBindingPoint(TransformsUniformBufferName), TransformsUniformBufferBindingPoint, _transformsBuffer.lock()->GetId());
  }

  if (!_skyBox.expired())
  {
    _skyBox.lock()->GetTexture()->BindToTextureSlot(SkyBoxTextureSlot);
    SetInt(SkyBoxUniformName, SkyBoxTextureSlot);
  }
}