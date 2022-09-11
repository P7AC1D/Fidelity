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

struct LightingConstantsBuffer
{
  Matrix4 ProjViewInvs;
  Vector3 CameraPosition;
  float32 _padding;
  Vector2 PixelSize;
};

struct LightData
{
  Vector3 Colour = Vector3::Zero;
  float32 Intensity = 0.0f;
  Vector3 Position = Vector3::Zero;
  float32 Radius = 0.0f;
  Vector3 Direction = Vector3::Zero;
  int32 LightType = 0;
};

static const int32 MAX_LIGHTS = 32;
struct LightingBuffer
{
  Vector3 AmbientColour;
  float32 AmbientIntensity;
  LightData Lights[MAX_LIGHTS];
};

DeferredRenderer::DeferredRenderer(const Vector2I &windowDims) : _windowDims(windowDims),
                                                                 _ambientColour(Colour::White),
                                                                 _ambientIntensity(0.1f)
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
    vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/Lighting.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("LightingConstantsBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->AddParam(ShaderParam("LightingBuffer", ShaderParamType::ConstBuffer, 1));
    shaderParams->AddParam(ShaderParam("AlbedoMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 2));
    shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 3));

    RasterizerStateDesc rasterizerStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    BlendStateDesc blendStateDesc{};

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _lightingPto = device->CreatePipelineState(pipelineDesc);
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
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _lightingPassRto = device->CreateRenderTarget(rtDesc);
  }
  GpuBufferDesc lightingConstantsBufferDesc;
  lightingConstantsBufferDesc.BufferType = BufferType::Constant;
  lightingConstantsBufferDesc.BufferUsage = BufferUsage::Stream;
  lightingConstantsBufferDesc.ByteCount = sizeof(LightingConstantsBuffer);
  _lightingConstantsBuffer = device->CreateGpuBuffer(lightingConstantsBufferDesc);

  GpuBufferDesc lightingBufferDesc;
  lightingBufferDesc.BufferType = BufferType::Constant;
  lightingBufferDesc.BufferUsage = BufferUsage::Stream;
  lightingBufferDesc.ByteCount = sizeof(LightingBuffer);
  _lightingBuffer = device->CreateGpuBuffer(lightingBufferDesc);
}

void DeferredRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Deferred Renderer");

    float32 rawCol[]{_ambientColour[0], _ambientColour[1], _ambientColour[2]};
    ImGui::ColorEdit3("Ambient Colour", rawCol);
    _ambientColour = Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255);

    ImGui::DragFloat("Ambient Intensity", &_ambientIntensity, 0.01f, 0.0f, 1.0f);
  }
}

void DeferredRenderer::drawFrame(std::shared_ptr<RenderDevice> renderDevice,
                                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                                 const std::vector<std::shared_ptr<Light>> &lights,
                                 const Camera &camera)
{
  renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  renderDevice->SetViewport(viewportDesc);

  gbufferPass(renderDevice, drawables, camera);
  lightingPass(renderDevice, lights, camera);
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

void DeferredRenderer::lightingPass(std::shared_ptr<RenderDevice> renderDevice,
                                    const std::vector<std::shared_ptr<Light>> &lights,
                                    const Camera &camera)
{
  renderDevice->SetPipelineState(_lightingPto);
  renderDevice->SetRenderTarget(_lightingPassRto);
  renderDevice->ClearBuffers(RTT_Colour);
  renderDevice->SetTexture(0, _gBufferRto->GetColourTarget(0));
  renderDevice->SetTexture(1, _gBufferRto->GetDepthStencilTarget());
  renderDevice->SetTexture(2, _gBufferRto->GetColourTarget(1));
  renderDevice->SetTexture(3, _gBufferRto->GetColourTarget(2));
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _noMipSamplerState);
  renderDevice->SetSamplerState(3, _noMipSamplerState);
  renderDevice->SetConstantBuffer(0, _lightingConstantsBuffer);
  renderDevice->SetConstantBuffer(1, _lightingBuffer);

  Transform cameraTransform(camera.getTransformCopy());
  LightingConstantsBuffer lightingConstants;
  lightingConstants.ProjViewInvs = (camera.getProj() * camera.getView()).Inverse();
  lightingConstants.CameraPosition = cameraTransform.getPosition();
  lightingConstants.PixelSize = Vector2(1.0f / _windowDims.X, 1.0f / _windowDims.Y);
  _lightingConstantsBuffer->WriteData(0, sizeof(LightingConstantsBuffer), &lightingConstants, AccessType::WriteOnlyDiscard);

  LightingBuffer lighting;
  lighting.AmbientColour = _ambientColour.ToVec3();
  lighting.AmbientIntensity = _ambientIntensity;
  for (uint32 i = 0; i < lights.size(); i++)
  {
    auto light = lights[i];
    LightData lightData;
    lightData.Colour = light->getColour().ToVec3();
    lightData.Direction = light->getDirection();
    lightData.Intensity = light->getIntensity();
    lightData.LightType = static_cast<int32>(light->getLightType());
    lightData.Position = light->getPosition();
    lightData.Radius = light->getRadius();
    lighting.Lights[i] = lightData;
  }
  _lightingBuffer->WriteData(0, sizeof(LightingBuffer), &lighting, AccessType::WriteOnlyDiscard);

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