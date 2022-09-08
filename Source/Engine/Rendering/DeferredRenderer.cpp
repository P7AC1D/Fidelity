#include "DeferredRenderer.h"

#include <iostream>

#include "../Geometry/MeshFactory.h"
#include "../RenderApi/BlendState.hpp"
#include "../RenderApi/DepthStencilState.hpp"
#include "../RenderApi/Shader.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/RasterizerState.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../RenderApi/Texture.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../RenderApi/VertexLayout.hpp"
#include "../Utility/String.hpp"
#include "../UI/ImGui/imgui.h"
#include "Camera.h"
#include "Drawable.h"
#include "Light.h"
#include "Material.h"
#include "StaticMesh.h"

struct MaterialBufferData
{
  struct TextureMapFlagData
  {
    int32 Diffuse = 0;
    int32 Normal = 0;
    int32 Specular = 0;
    int32 Opacity = 0;
  } EnabledTextureMaps;
  Colour Ambient = Colour::White;
  Colour Diffuse = Colour::White;
  Colour Specular = Colour::White;
  float32 Exponent = 1.0f;
  int32 GreyscaleSpecular = 1;
};

struct PointLightConstantsBuffer
{
  Matrix4 ProjViewInvs;
  Vector3 CameraPosition;
  float32 _padding;
  Vector2 PixelSize;
};

struct PointLightBuffer
{
  Matrix4 Model;
  Matrix4 ModelView;
  Matrix4 ModelViewProjection;
  Vector4 Colour;
  Vector3 Position;
  float32 Radius;
};

DeferredRenderer::DeferredRenderer(const Vector2I &windowDims) : _windowDims(windowDims)
{
}

void DeferredRenderer::onInit(const std::shared_ptr<RenderDevice> &device)
{
  {
    ShaderDesc vsDesc;
    vsDesc.EntryPoint = "main";
    vsDesc.ShaderLang = ShaderLang::Glsl;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::LoadFromFile("./Shaders/Gbuffer.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/Gbuffer.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->AddParam(ShaderParam("MaterialBuffer", ShaderParamType::ConstBuffer, 2));
    shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));
    shaderParams->AddParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 3));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

    BlendStateDesc blendStateDesc{};
    blendStateDesc.RTBlendState[0].BlendEnabled = true;
    blendStateDesc.RTBlendState[0].BlendAlpha = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _gBufferPso = device->CreatePipelineState(pipelineDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::RGBA8;

    TextureDesc depthStencilDesc;
    depthStencilDesc.Width = _windowDims.X;
    depthStencilDesc.Height = _windowDims.Y;
    depthStencilDesc.Usage = TextureUsage::Depth;
    depthStencilDesc.Type = TextureType::Texture2D;
    depthStencilDesc.Format = TextureFormat::D24;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.ColourTargets[1] = device->CreateTexture(colourTexDesc);
    rtDesc.ColourTargets[2] = device->CreateTexture(colourTexDesc);
    rtDesc.DepthStencilTarget = device->CreateTexture(depthStencilDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _gBufferRto = device->CreateRenderTarget(rtDesc);
  }

  GpuBufferDesc materialBufferDesc;
  materialBufferDesc.BufferType = BufferType::Constant;
  materialBufferDesc.BufferUsage = BufferUsage::Stream;
  materialBufferDesc.ByteCount = sizeof(MaterialBufferData);
  _materialBuffer = device->CreateGpuBuffer(materialBufferDesc);

  GpuBufferDesc objectBufferDesc;
  objectBufferDesc.BufferType = BufferType::Constant;
  objectBufferDesc.BufferUsage = BufferUsage::Stream;
  objectBufferDesc.ByteCount = sizeof(ObjectBuffer);
  _objectBuffer = device->CreateGpuBuffer(objectBufferDesc);

  {
    ShaderDesc vsDesc;
    vsDesc.EntryPoint = "main";
    vsDesc.ShaderLang = ShaderLang::Glsl;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::LoadFromFile("./Shaders/PointLights.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/PointLights.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
        VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->AddParam(ShaderParam("PointLightPassConstants", ShaderParamType::ConstBuffer, 1));
    shaderParams->AddParam(ShaderParam("PointLightBuffer", ShaderParamType::ConstBuffer, 2));
    shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));

    RasterizerStateDesc rasterizerStateDesc{};
    rasterizerStateDesc.CullMode = CullMode::Clockwise;

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    BlendStateDesc blendStateDesc{};
    blendStateDesc.RTBlendState[0].BlendEnabled = true;
    blendStateDesc.RTBlendState[0].Blend = BlendDesc(BlendFactor::One, BlendFactor::One, BlendOperation::Add);

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _lightPrePassCWPto = device->CreatePipelineState(pipelineDesc);

    rasterizerStateDesc.CullMode = CullMode::CounterClockwise;
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    _lightPrePassCCWPto = device->CreatePipelineState(pipelineDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::RGB8;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.ColourTargets[1] = device->CreateTexture(colourTexDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _lightPrePassRto = device->CreateRenderTarget(rtDesc);
  }
  GpuBufferDesc pointLightConstantsBufferDesc;
  pointLightConstantsBufferDesc.BufferType = BufferType::Constant;
  pointLightConstantsBufferDesc.BufferUsage = BufferUsage::Stream;
  pointLightConstantsBufferDesc.ByteCount = sizeof(PointLightConstantsBuffer);
  _pointLightConstantsBuffer = device->CreateGpuBuffer(pointLightConstantsBufferDesc);

  GpuBufferDesc pointLightBufferDesc;
  pointLightBufferDesc.BufferType = BufferType::Constant;
  pointLightBufferDesc.BufferUsage = BufferUsage::Stream;
  pointLightBufferDesc.ByteCount = sizeof(PointLightBuffer);
  _pointLightBuffer = device->CreateGpuBuffer(pointLightBufferDesc);

  _pointLightMesh = MeshFactory::CreateUvSphere();

  {
    ShaderDesc vsDesc;
    vsDesc.EntryPoint = "main";
    vsDesc.ShaderLang = ShaderLang::Glsl;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/Merge.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("EmissiveMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _mergePso = device->CreatePipelineState(pipelineDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::RGB8;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.ColourTargets[1] = device->CreateTexture(colourTexDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _mergePassRto = device->CreateRenderTarget(rtDesc);
  }
}

void DeferredRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Deferred Renderer");
  }  
}

void DeferredRenderer::drawFrame(std::shared_ptr<RenderDevice> renderDevice,
                                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                                 const std::vector<std::shared_ptr<Light>> &lights,
                                 const Camera &camera)
{
  renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);

  gbufferPass(renderDevice, drawables, camera);
  lightPrePass(renderDevice, lights, camera);
  mergePass(renderDevice);
}

void DeferredRenderer::gbufferPass(std::shared_ptr<RenderDevice> device,
                                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                                   const Camera &camera)
{
  device->SetPipelineState(_gBufferPso);
  device->SetRenderTarget(_gBufferRto);
  device->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);

  for (auto drawable : drawables)
  {
    writeMaterialConstantData(device, drawable->getMaterial());
    writeObjectConstantData(drawable, camera);

    device->SetConstantBuffer(0, _objectBuffer);
    device->SetConstantBuffer(2, _materialBuffer);

    auto mesh = drawable->getMesh();
    device->SetVertexBuffer(mesh->getVertexData(device));

    if (mesh->isIndexed())
    {
      auto indexCount = mesh->getIndexCount();
      device->SetIndexBuffer(mesh->getIndexData(device));
      device->DrawIndexed(indexCount, 0, 0);
    }
    else
    {
      device->Draw(mesh->getVertexCount(), 0);
    }
  }
}

void DeferredRenderer::lightPrePass(std::shared_ptr<RenderDevice> renderDevice,
                                    const std::vector<std::shared_ptr<Light>> &lights,
                                    const Camera &camera)
{
  renderDevice->SetRenderTarget(_lightPrePassRto);
  renderDevice->ClearBuffers(RTT_Colour);
  renderDevice->SetTexture(0, _gBufferRto->GetDepthStencilTarget());
  renderDevice->SetTexture(1, _gBufferRto->GetColourTarget(1));
  renderDevice->SetTexture(2, _gBufferRto->GetColourTarget(2));
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _noMipSamplerState);
  renderDevice->SetConstantBuffer(1, _pointLightConstantsBuffer);
  renderDevice->SetConstantBuffer(2, _pointLightBuffer);

  Transform cameraTransform(camera.getTransformCopy());
  PointLightConstantsBuffer pointLightConstantsBuffer;
  pointLightConstantsBuffer.ProjViewInvs = (camera.getProj() * camera.getView()).Inverse();
  pointLightConstantsBuffer.CameraPosition = cameraTransform.getPosition();
  pointLightConstantsBuffer.PixelSize = Vector2(1.0f / _windowDims.X, 1.0f / _windowDims.Y);
  _pointLightConstantsBuffer->WriteData(0, sizeof(PointLightConstantsBuffer), &pointLightConstantsBuffer, AccessType::WriteOnlyDiscard);

  for (auto light : lights)
  {
    float32 distToLight = (light->getPosition() - cameraTransform.getPosition()).Length();
    if (distToLight < light->getRadius())
    {
      renderDevice->SetPipelineState(_lightPrePassCWPto);
    }
    else
    {
      renderDevice->SetPipelineState(_lightPrePassCCWPto);
    }

    PointLightBuffer pointLightBuffer;
    pointLightBuffer.Model = light->getMatrix();
    pointLightBuffer.ModelView = camera.getView() * pointLightBuffer.Model;
    pointLightBuffer.ModelViewProjection = camera.getProj() * pointLightBuffer.ModelView;
    pointLightBuffer.Colour = light->getColour().ToVec3();
    pointLightBuffer.Position = light->getPosition();
    pointLightBuffer.Radius = light->getRadius();
    _pointLightBuffer->WriteData(0, sizeof(PointLightBuffer), &pointLightBuffer, AccessType::WriteOnlyDiscard);

    renderDevice->SetConstantBuffer(0, _pointLightBuffer);
    renderDevice->SetVertexBuffer(_pointLightMesh->getVertexData(renderDevice));
    renderDevice->SetIndexBuffer(_pointLightMesh->getIndexData(renderDevice));
    renderDevice->DrawIndexed(_pointLightMesh->getIndexCount(), 0, 0);
  }
}

void DeferredRenderer::mergePass(std::shared_ptr<RenderDevice> renderDevice)
{
  renderDevice->SetRenderTarget(_mergePassRto);
  renderDevice->SetPipelineState(_mergePso);
  renderDevice->SetTexture(0, _gBufferRto->GetColourTarget(0));
  renderDevice->SetTexture(1, _lightPrePassRto->GetColourTarget(0));
  renderDevice->SetTexture(2, _lightPrePassRto->GetColourTarget(1));
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _noMipSamplerState);
  renderDevice->SetVertexBuffer(_fsQuadBuffer);
  renderDevice->Draw(6, 0);
}

void DeferredRenderer::writeMaterialConstantData(std::shared_ptr<RenderDevice> renderDevice, std::shared_ptr<Material> material) const
{
  MaterialBufferData matData;
  matData.Ambient = material->getAmbientColour();
  matData.Diffuse = material->getDiffuseColour();
  matData.Specular = material->getSpecularColour();
  matData.Exponent = material->getSpecularExponent();

  auto diffuseTexture = material->getDiffuseTexture();
  if (diffuseTexture)
  {
    matData.EnabledTextureMaps.Diffuse = 1;
    renderDevice->SetTexture(0, diffuseTexture);
    renderDevice->SetSamplerState(0, _basicSamplerState);
  }

  auto normalTexture = material->getNormalTexture();
  if (normalTexture)
  {
    matData.EnabledTextureMaps.Normal = 1;
    renderDevice->SetTexture(1, normalTexture);
    renderDevice->SetSamplerState(1, _noMipSamplerState);
  }

  auto specularTexture = material->getSpecularTexture();
  if (specularTexture)
  {
    matData.EnabledTextureMaps.Specular = 1;
    renderDevice->SetTexture(2, specularTexture);
    renderDevice->SetSamplerState(2, _noMipSamplerState);
  }

  _materialBuffer->WriteData(0, sizeof(MaterialBufferData), &matData, AccessType::WriteOnlyDiscard);
}

void DeferredRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const Camera &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera.getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera.getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}