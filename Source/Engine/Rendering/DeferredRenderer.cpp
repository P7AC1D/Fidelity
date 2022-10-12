#include "DeferredRenderer.h"

#include <chrono>
#include <iostream>
#include <random>

#include "../Geometry/MeshFactory.h"
#include "../Maths/Math.hpp"
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

const static uint32 RANDOM_ROTATION_TEXTURE_SIZE = 64;
const static uint32 SSAO_NOISE_TEXTURE_SIZE = 4;
const static uint32 SSAO_MAX_KERNAL_SIZE = 512;
struct SsaoConstantsData
{
  Matrix4 View;
  Matrix4 Projection;
  Matrix4 ProjectionInv;
  Vector4 NoiseSamples[SSAO_MAX_KERNAL_SIZE];
  uint32 KernelSize;
  float32 Radius;
  float32 Bias;
};

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
  Matrix4 View;
  Matrix4 ProjViewInvs;
  Vector3 CameraPosition;
  float32 FarPlane;
  int32 SsaoEnabled;
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
                                                                 _ambientIntensity(0.3f),
                                                                 _ssaoSamples(64),
                                                                 _ssaoBias(0.0f),
                                                                 _ssaoRadius(0.5f),
                                                                 _ssaoEnabled(true)
{
  _renderPassTimings.push_back({0, "G-Buffer"});
  _renderPassTimings.push_back({0, "Shadow Merge"});
  _renderPassTimings.push_back({0, "SSAO"});
  _renderPassTimings.push_back({0, "Lighting"});
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
    ShaderDesc vsDesc;
    vsDesc.EntryPoint = "main";
    vsDesc.ShaderLang = ShaderLang::Glsl;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/Shadows.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("LightingConstantsBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->AddParam(ShaderParam("CascadeShadowMapBuffer", ShaderParamType::ConstBuffer, 2));
    shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("ShadowMap", ShaderParamType::Texture, 2));
    shaderParams->AddParam(ShaderParam("RandomRotationsMap", ShaderParamType::Texture, 3));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _shadowsPso = device->CreatePipelineState(pipelineDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::R8;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _shadowsRto = device->CreateRenderTarget(rtDesc);
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
    colourTexDesc.Format = TextureFormat::RGB16F;
    rtDesc.ColourTargets[1] = device->CreateTexture(colourTexDesc);
    colourTexDesc.Format = TextureFormat::RGBA8;
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
    shaderParams->AddParam(ShaderParam("CascadeShadowMapBuffer", ShaderParamType::ConstBuffer, 2));

    shaderParams->AddParam(ShaderParam("AlbedoMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 2));
    shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 3));
    shaderParams->AddParam(ShaderParam("ShadowMap", ShaderParamType::Texture, 4));
    shaderParams->AddParam(ShaderParam("OcculusionMap", ShaderParamType::Texture, 5));

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

  SamplerStateDesc shadowMapSamplerStateDesc;
  shadowMapSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Border, TextureAddressMode::Border, TextureAddressMode::Border};
  shadowMapSamplerStateDesc.MinFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.MinFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.MipFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.BorderColour = Colour::White;
  _shadowMapSamplerState = device->CreateSamplerState(shadowMapSamplerStateDesc);

  TextureDesc randomRotationsDesc;
  randomRotationsDesc.Width = RANDOM_ROTATION_TEXTURE_SIZE;
  randomRotationsDesc.Height = RANDOM_ROTATION_TEXTURE_SIZE;
  randomRotationsDesc.Usage = TextureUsage::Default;
  randomRotationsDesc.Type = TextureType::Texture2D;
  randomRotationsDesc.Format = TextureFormat::R8;
  _randomRotationsMap = device->CreateTexture(randomRotationsDesc);

  ubyte randomValues[RANDOM_ROTATION_TEXTURE_SIZE * RANDOM_ROTATION_TEXTURE_SIZE];
  srand(0);
  for (uint32 i = 0; i < RANDOM_ROTATION_TEXTURE_SIZE * RANDOM_ROTATION_TEXTURE_SIZE; i++)
  {
    randomValues[i] = static_cast<ubyte>((rand() / static_cast<float32>(RAND_MAX)) * 255.0f);
  }

  std::shared_ptr<ImageData> imageData(new ImageData(RANDOM_ROTATION_TEXTURE_SIZE, RANDOM_ROTATION_TEXTURE_SIZE, 1, ImageFormat::R8));
  imageData->WriteData(randomValues);
  _randomRotationsMap->WriteData(0, 0, imageData);

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
    psDesc.Source = String::LoadFromFile("./Shaders/Ssao.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("SsaoConstantsBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
    shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->AddParam(ShaderParam("NoiseMap", ShaderParamType::Texture, 2));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _ssaoPso = device->CreatePipelineState(pipelineDesc);
  }
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
    psDesc.Source = String::LoadFromFile("./Shaders/SsaoBlur.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("SsaoMap", ShaderParamType::Texture, 0));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = device->CreateShader(vsDesc);
    pipelineDesc.PS = device->CreateShader(psDesc);
    pipelineDesc.BlendState = device->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = device->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = device->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = device->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _ssaoBlurPso = device->CreatePipelineState(pipelineDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::R8;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _ssaoRto = device->CreateRenderTarget(rtDesc);
  }
  {
    TextureDesc colourTexDesc;
    colourTexDesc.Width = _windowDims.X;
    colourTexDesc.Height = _windowDims.Y;
    colourTexDesc.Usage = TextureUsage::RenderTarget;
    colourTexDesc.Type = TextureType::Texture2D;
    colourTexDesc.Format = TextureFormat::R8;

    RenderTargetDesc rtDesc;
    rtDesc.ColourTargets[0] = device->CreateTexture(colourTexDesc);
    rtDesc.Height = _windowDims.X;
    rtDesc.Width = _windowDims.Y;

    _ssaoBlurRto = device->CreateRenderTarget(rtDesc);
  }

  TextureDesc ssaoNoiseTextureDesc;
  ssaoNoiseTextureDesc.Width = SSAO_NOISE_TEXTURE_SIZE;
  ssaoNoiseTextureDesc.Height = SSAO_NOISE_TEXTURE_SIZE;
  ssaoNoiseTextureDesc.Usage = TextureUsage::Default;
  ssaoNoiseTextureDesc.Type = TextureType::Texture2D;
  ssaoNoiseTextureDesc.Format = TextureFormat::RGB32F;
  _ssaoNoiseTexture = device->CreateTexture(ssaoNoiseTextureDesc);
  writeSsaoNoiseTexture(device);

  SamplerStateDesc ssaoNoiseSamplerDesc;
  ssaoNoiseSamplerDesc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
  ssaoNoiseSamplerDesc.MinFiltering = TextureFilteringMode::None;
  ssaoNoiseSamplerDesc.MinFiltering = TextureFilteringMode::None;
  ssaoNoiseSamplerDesc.MipFiltering = TextureFilteringMode::None;
  _ssaoNoiseSampler = device->CreateSamplerState(ssaoNoiseSamplerDesc);

  GpuBufferDesc ssaoConstantsDataDesc;
  ssaoConstantsDataDesc.BufferType = BufferType::Constant;
  ssaoConstantsDataDesc.BufferUsage = BufferUsage::Stream;
  ssaoConstantsDataDesc.ByteCount = sizeof(SsaoConstantsData);
  _ssaoConstantsBuffer = device->CreateGpuBuffer(ssaoConstantsDataDesc);
}

void DeferredRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Ambient Lighting");

    float32 rawCol[]{_ambientColour[0], _ambientColour[1], _ambientColour[2]};
    if (ImGui::ColorEdit3("Colour", rawCol))
    {
      _ambientColour = Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255);
    }

    float32 ambientIntensity = _ambientIntensity;
    if (ImGui::SliderFloat("Intensity", &ambientIntensity, 0.0f, 1.0f))
    {
      _ambientIntensity = ambientIntensity;
    }
  }
  ImGui::Separator();
  {
    ImGui::Text("Ambient Occlusion");

    float32 ssaoRadius = _ssaoRadius;
    if (ImGui::SliderFloat("Radius", &ssaoRadius, 0.1f, 1.0f))
    {
      _ssaoRadius = ssaoRadius;
    }

    float32 ssaoBias = _ssaoBias;
    if (ImGui::SliderFloat("Bias", &ssaoBias, 0.0f, 0.055f))
    {
      _ssaoBias = ssaoBias;
    }

    int32 ssaoSamples = _ssaoSamples;
    if (ImGui::SliderInt("Samples", &ssaoSamples, 1, SSAO_MAX_KERNAL_SIZE))
    {
      _ssaoSamples = ssaoSamples;
    }

    bool ssaoEnabled = _ssaoEnabled;
    if (ImGui::Checkbox("Enabled", &ssaoEnabled))
    {
      _ssaoEnabled = ssaoEnabled;
    }
  }
}

void DeferredRenderer::drawFrame(std::shared_ptr<RenderDevice> renderDevice,
                                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                                 const std::vector<std::shared_ptr<Light>> &lights,
                                 const std::shared_ptr<RenderTarget> &shadowMapRto,
                                 const std::shared_ptr<GpuBuffer> &shadowMapBuffer,
                                 const std::shared_ptr<Camera> &camera)
{
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  renderDevice->SetViewport(viewportDesc);

  gbufferPass(renderDevice, drawables, camera);

  LightingConstantsBuffer lightingConstants;
  lightingConstants.View = camera->getView();
  lightingConstants.FarPlane = camera->getFar();
  lightingConstants.ProjViewInvs = (camera->getProj() * camera->getView()).Inverse();
  lightingConstants.CameraPosition = camera->getParentTransform().getPosition();
  lightingConstants.SsaoEnabled = _ssaoEnabled;
  _lightingConstantsBuffer->WriteData(0, sizeof(LightingConstantsBuffer), &lightingConstants, AccessType::WriteOnlyDiscard);

  shadowPass(renderDevice, shadowMapRto, shadowMapBuffer);
  ssaoPass(renderDevice, camera);
  lightingPass(renderDevice, lights, shadowMapRto, shadowMapBuffer, camera);
}

void DeferredRenderer::gbufferPass(std::shared_ptr<RenderDevice> device,
                                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                                   const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

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

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[0].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void DeferredRenderer::shadowPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                  const std::shared_ptr<RenderTarget> &shadowMapRto,
                                  const std::shared_ptr<GpuBuffer> &shadowMapBuffer)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->SetPipelineState(_shadowsPso);
  renderDevice->SetRenderTarget(_shadowsRto);
  renderDevice->SetTexture(0, _gBufferRto->GetDepthStencilTarget());
  renderDevice->SetTexture(1, _gBufferRto->GetColourTarget(1));
  renderDevice->SetTexture(2, shadowMapRto->GetDepthStencilTarget());
  renderDevice->SetTexture(3, _randomRotationsMap);
  renderDevice->SetConstantBuffer(0, _lightingConstantsBuffer);
  renderDevice->SetConstantBuffer(2, shadowMapBuffer);
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _shadowMapSamplerState);
  renderDevice->SetSamplerState(3, _noMipSamplerState);
  renderDevice->SetVertexBuffer(_fsQuadBuffer);
  renderDevice->Draw(6, 0);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[1].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void DeferredRenderer::ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  writeSsaoConstantData(renderDevice, camera);

  renderDevice->SetPipelineState(_ssaoPso);
  renderDevice->SetRenderTarget(_ssaoRto);
  renderDevice->ClearBuffers(RTT_Colour);
  renderDevice->SetTexture(0, _gBufferRto->GetDepthStencilTarget());
  renderDevice->SetTexture(1, _gBufferRto->GetColourTarget(1));
  renderDevice->SetTexture(2, _ssaoNoiseTexture);
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _ssaoNoiseSampler);
  renderDevice->SetConstantBuffer(0, _ssaoConstantsBuffer);
  renderDevice->SetVertexBuffer(_fsQuadBuffer);
  renderDevice->Draw(6, 0);

  renderDevice->SetPipelineState(_ssaoBlurPso);
  renderDevice->SetRenderTarget(_ssaoBlurRto);
  renderDevice->ClearBuffers(RTT_Colour);
  renderDevice->SetTexture(0, _ssaoRto->GetColourTarget(0));
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetVertexBuffer(_fsQuadBuffer);
  renderDevice->Draw(6, 0);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[2].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void DeferredRenderer::lightingPass(std::shared_ptr<RenderDevice> renderDevice,
                                    const std::vector<std::shared_ptr<Light>> &lights,
                                    const std::shared_ptr<RenderTarget> &shadowMapRto,
                                    const std::shared_ptr<GpuBuffer> &shadowMapBuffer,
                                    const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->SetPipelineState(_lightingPto);
  renderDevice->SetRenderTarget(_lightingPassRto);
  renderDevice->SetTexture(0, _gBufferRto->GetColourTarget(0));
  renderDevice->SetTexture(1, _gBufferRto->GetDepthStencilTarget());
  renderDevice->SetTexture(2, _gBufferRto->GetColourTarget(1));
  renderDevice->SetTexture(3, _gBufferRto->GetColourTarget(2));
  renderDevice->SetTexture(4, _shadowsRto->GetColourTarget(0));
  renderDevice->SetTexture(5, _ssaoBlurRto->GetColourTarget(0));
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetSamplerState(1, _noMipSamplerState);
  renderDevice->SetSamplerState(2, _noMipSamplerState);
  renderDevice->SetSamplerState(3, _noMipSamplerState);
  renderDevice->SetSamplerState(4, _noMipSamplerState);
  renderDevice->SetSamplerState(5, _noMipSamplerState);
  renderDevice->SetConstantBuffer(0, _lightingConstantsBuffer);
  renderDevice->SetConstantBuffer(1, _lightingBuffer);
  renderDevice->SetConstantBuffer(2, shadowMapBuffer);

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

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[3].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
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

void DeferredRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}

void DeferredRenderer::writeSsaoNoiseTexture(const std::shared_ptr<RenderDevice> &renderDevice) const
{
  std::uniform_real_distribution<float32> randomFloats(0.0, 1.0);
  std::default_random_engine generator;

  std::vector<Vector3> ssaoNoise;
  for (uint32 i = 0; i < SSAO_NOISE_TEXTURE_SIZE * SSAO_NOISE_TEXTURE_SIZE; i++)
  {
    Vector3 noise(randomFloats(generator) * 2.0 - 1.0,
                  randomFloats(generator) * 2.0 - 1.0,
                  0.0f);
    ssaoNoise.push_back(noise);
  }

  _ssaoNoiseTexture->WriteData(0, 0, 0, SSAO_NOISE_TEXTURE_SIZE, 0, SSAO_NOISE_TEXTURE_SIZE, 0, 0, ssaoNoise.data());
}

void DeferredRenderer::writeSsaoConstantData(const std::shared_ptr<RenderDevice> &renderDevice,
                                             const std::shared_ptr<Camera> &camera) const
{
  std::uniform_real_distribution<float32> randomFloats(0.0, 1.0);
  std::default_random_engine generator;
  std::vector<Vector3> ssaoKernel;

  for (uint32 i = 0; i < SSAO_MAX_KERNAL_SIZE; ++i)
  {
    Vector3 sample(randomFloats(generator) * 2.0 - 1.0,
                   randomFloats(generator) * 2.0 - 1.0,
                   randomFloats(generator));

    sample.Normalize();
    sample *= randomFloats(generator);
    float32 scale = float32(i) / _ssaoSamples;

    // scale samples to be more centered around the center of kernel
    scale = Math::Lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }

  SsaoConstantsData ssaoConstantsData;
  ssaoConstantsData.View = camera->getView();
  ssaoConstantsData.Projection = camera->getProj();
  ssaoConstantsData.ProjectionInv = camera->getProj().Inverse();
  ssaoConstantsData.Bias = _ssaoBias;
  ssaoConstantsData.Radius = _ssaoRadius;
  ssaoConstantsData.KernelSize = _ssaoSamples;
  for (uint32 i = 0; i < SSAO_MAX_KERNAL_SIZE; ++i)
  {
    ssaoConstantsData.NoiseSamples[i] = ssaoKernel[i];
  }
  _ssaoConstantsBuffer->WriteData(0, sizeof(SsaoConstantsData), &ssaoConstantsData, AccessType::WriteOnlyDiscard);
}