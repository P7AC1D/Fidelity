#include "Renderer.h"

#include <chrono>
#include <iostream>
#include <random>

#include "../Core/Maths.h"
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
#include "../UI/ImGui/imgui.h"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "Camera.h"
#include "Drawable.h"
#include "Material.h"
#include "Light.h"
#include "StaticMesh.h"

const static uint32 RANDOM_ROTATION_TEXTURE_SIZE = 64;
const static uint32 SSAO_NOISE_TEXTURE_SIZE = 4;
const static uint32 SSAO_MAX_KERNAL_SIZE = 512;
const static uint32 MAX_LIGHTS = 1024;
const static uint32 MAX_CASCADE_LAYERS = 8;

struct SsaoConstantsData
{
  Vector4 NoiseSamples[SSAO_MAX_KERNAL_SIZE];
  uint32 KernelSize;
  float32 Radius;
  float32 Bias;
};

struct PerObjectBufferData
{
  Matrix4 Model;
  Matrix4 ModelView;
  Matrix4 ModelViewProjection;
  int32 DiffuseEnabled = 0;
  int32 NormalEnabled = 0;
  int32 SpecularEnabled = 0;
  int32 OpacityEnabled = 0;
  Colour AmbientColour;
  Colour DiffuseColour;
  Colour SpecularColour;
  float32 SpecularExponent;
};

struct LightData
{
  Vector3 Colour = Vector3::Zero;
  float32 Intensity = 0.0f;
  Vector3 Position = Vector3::Zero;
  float32 Radius = 0.0f;
};

struct PerFrameBufferData
{
  Matrix4 CascadeLightTransforms[MAX_CASCADE_LAYERS];
  Matrix4 View;
  Matrix4 Proj;
  Matrix4 ProjInv;
  Matrix4 ProjViewInv;
  // --------- Alignment ----------
  Vector3 ViewPosition;
  float32 FarPlane;
  // --------- Alignment ----------
  Vector3 LightDirection; // ---- Directional Light ----
  int32 SsaoEnabled;
  // --------- Alignment ----------
  Vector3 LightColour;    // ---- Directional Light ----
  float32 LightIntensity; // ---- Directional Light ----
                          // --------- Alignment ----------
  Vector3 AmbientColour;
  float32 AmbientIntensity;
  // --------- Alignment ----------
  uint32 CascadeLayerCount;
  int32 DrawCascadeLayers;
  uint32 ShadowSampleCount;
  float32 ShadowSampleSpread;
  // --------- Alignment ----------
  LightData Lights[MAX_LIGHTS];
  // --------- Alignment ----------
  Vector4 CascadePlaneDistances[MAX_CASCADE_LAYERS];
  // --------- Alignment ----------
  uint32 LightCount;
  Vector3 __Padding;
};

struct ShadowMapDebugData
{
  float32 NearClip;
  float32 FarClip;
  uint32 Layer;
};

struct FullscreenQuadData
{
  int32 SingleChannel;
};

struct FullscreenQuadVertex
{
  Vector2 Position;
  Vector2 TexCoord;

  FullscreenQuadVertex(const Vector2 &position, const Vector2 &texCoord) : Position(position), TexCoord(texCoord)
  {
  }
};

std::vector<Vector3> AabbCoords = {
    Vector3(-1.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0),
    Vector3(1.0, 1.0, 1.0), Vector3(1.0, -1.0, 1.0),
    Vector3(1.0, -1.0, 1.0), Vector3(-1.0, -1.0, 1.0),
    Vector3(-1.0, -1.0, 1.0), Vector3(-1.0, 1.0, 1.0),

    Vector3(-1.0, 1.0, 1.0), Vector3(-1.0, 1.0, -1.0),
    Vector3(-1.0, 1.0, -1.0), Vector3(1.0, 1.0, -1.0),
    Vector3(1.0, 1.0, -1.0), Vector3(1.0, 1.0, 1.0),

    Vector3(1.0, 1.0, -1.0), Vector3(1.0, -1.0, -1.0),
    Vector3(1.0, -1.0, -1.0), Vector3(1.0, -1.0, 1.0),

    Vector3(-1.0, 1.0, -1.0), Vector3(-1.0, -1.0, -1.0),
    Vector3(-1.0, -1.0, -1.0), Vector3(-1.0, -1.0, 1.0),

    Vector3(-1.0, -1.0, -1.0), Vector3(1.0, -1.0, -1.0)};

std::vector<FullscreenQuadVertex> FullscreenQuadVertices{
    FullscreenQuadVertex(Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)),
    FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
    FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f)),
    FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
    FullscreenQuadVertex(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f)),
    FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f))};

float32 calculateCascadeRadius(const std::vector<Vector3> &frustrumCorners, const Vector3 &frustrumCenter)
{
  Assert::throwIfFalse(frustrumCorners.size() == 8, "Invalid size of supplied frustrum corners.");

  float32 sphereRadius = 0.0f;
  for (uint32 i = 0; i < 8; i++)
  {
    float32 dist = (frustrumCorners[i] - frustrumCenter).Length();
    sphereRadius = std::max(sphereRadius, dist);
  }

  sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
  return sphereRadius;
}

Vector3 calculateFrustrumCenter(const std::vector<Vector3> &frustrumCorners)
{
  Vector3 center(Vector3::Zero);
  for (uint32 i = 0; i < 8; ++i)
  {
    center = center + frustrumCorners[i];
  }
  return center * (1.0f / 8.0f);
}

std::vector<Vector3> calculateFrustrumCorners(const Matrix4 &view, const Matrix4 &projection)
{
  std::vector<Vector4> frustrumCornersVS = {
      Vector4(-1.0f, 1.0f, 0.0f, 1.0f),
      Vector4(1.0f, 1.0f, 0.0f, 1.0f),
      Vector4(1.0f, -1.0f, 0.0f, 1.0f),
      Vector4(-1.0f, -1.0f, 0.0f, 1.0f),
      Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
      Vector4(1.0f, 1.0f, 1.0f, 1.0f),
      Vector4(1.0f, -1.0f, 1.0f, 1.0f),
      Vector4(-1.0f, -1.0f, 1.0f, 1.0f)};

  Matrix4 projView(projection * view);
  Matrix4 projViewInvs(projView.Inverse());

  std::vector<Vector3> frustrumCornersWS;
  for (uint32 i = 0; i < 8; i++)
  {
    Vector4 point = projViewInvs * frustrumCornersVS[i];
    point = point / point.W;

    frustrumCornersWS.push_back(Vector3(point.X, point.Y, point.Z));
  }

  return frustrumCornersWS;
}

Renderer::Renderer(const Vector2I &windowDims) : _windowDims(windowDims),
                                                 _ambientColour(Colour::White),
                                                 _ambientIntensity(0.3f),
                                                 _ssaoSamples(64),
                                                 _ssaoBias(0.0f),
                                                 _ssaoRadius(0.5f),
                                                 _ssaoEnabled(true),
                                                 _drawCascadeLayers(false),
                                                 _shadowResolutionChanged(true),
                                                 _shadowMapResolution(2048),
                                                 _cascadeCount(4),
                                                 _shadowSampleCount(16),
                                                 _shadowSampleSpread(700.0f),
                                                 _minCascadeDistance(0.0f),
                                                 _maxCascadeDistance(1.0f),
                                                 _cascadeLambda(0.4f),
                                                 _debugDisplayType(DebugDisplayType::Disabled),
                                                 _shadowMapLayerToDraw(0),
                                                 _ssaoSettingsModified(true)

{
  _renderPassTimings.push_back({0, "Shadow Depth"});
  _renderPassTimings.push_back({0, "G-Buffer"});
  _renderPassTimings.push_back({0, "Transparency"});
  _renderPassTimings.push_back({0, "Shadow Merge"});
  _renderPassTimings.push_back({0, "SSAO"});
  _renderPassTimings.push_back({0, "Lighting"});
}

bool Renderer::init(const std::shared_ptr<RenderDevice> &renderDevice)
{
  try
  {
    VertexBufferDesc vtxBuffDesc;
    vtxBuffDesc.BufferUsage = BufferUsage::Default;
    vtxBuffDesc.VertexCount = FullscreenQuadVertices.size();
    vtxBuffDesc.VertexSizeBytes = sizeof(FullscreenQuadVertex);
    _fsQuadVertexBuffer = renderDevice->createVertexBuffer(vtxBuffDesc);
    _fsQuadVertexBuffer->writeData(0, sizeof(FullscreenQuadVertex) * FullscreenQuadVertices.size(), FullscreenQuadVertices.data(), AccessType::WriteOnlyDiscardRange);

    VertexBufferDesc aabbVertexBuffDesc;
    aabbVertexBuffDesc.BufferUsage = BufferUsage::Default;
    aabbVertexBuffDesc.VertexCount = AabbCoords.size();
    aabbVertexBuffDesc.VertexSizeBytes = sizeof(Vector3) * AabbCoords.size();
    _aabbVertexBuffer = renderDevice->createVertexBuffer(aabbVertexBuffDesc);
    _aabbVertexBuffer->writeData(0, sizeof(Vector3) * AabbCoords.size(), AabbCoords.data(), AccessType::WriteOnlyDiscardRange);

    initSamplers(renderDevice);
    initTextures(renderDevice);
    initConstantBuffers(renderDevice);

    initDirectionalLightDepthPass(renderDevice);
    initGbufferPass(renderDevice);
    initTransparencyPass(renderDevice);
    initShadowPass(renderDevice);
    initSsaoPass(renderDevice);
    initLightingPass(renderDevice);
    initDebugPass(renderDevice);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Failed to initialize Renderer. " << e.what() << std::endl;
    return false;
  }
  return true;
}

void Renderer::drawDebugUi()
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
      _ssaoSettingsModified = true;
    }

    float32 ssaoBias = _ssaoBias;
    if (ImGui::SliderFloat("Bias", &ssaoBias, 0.0f, 0.055f))
    {
      _ssaoBias = ssaoBias;
      _ssaoSettingsModified = true;
    }

    int32 ssaoSamples = _ssaoSamples;
    if (ImGui::SliderInt("Samples", &ssaoSamples, 1, SSAO_MAX_KERNAL_SIZE))
    {
      _ssaoSamples = ssaoSamples;
      _ssaoSettingsModified = true;
    }

    bool ssaoEnabled = _ssaoEnabled;
    if (ImGui::Checkbox("Enabled", &ssaoEnabled))
    {
      _ssaoEnabled = ssaoEnabled;
    }
  }

  ImGui::Separator();
  {
    ImGui::Text("Shadows");

    int shadowMapResolution = _shadowMapResolution;
    if (ImGui::SliderInt("Resolution", &shadowMapResolution, 256, 8192))
    {
      _shadowMapResolution = shadowMapResolution;
      _settingsModified = true;
    }

    int sampleCount = _shadowSampleCount;
    if (ImGui::SliderInt("Sample Count", &sampleCount, 1, 64))
    {
      _shadowSampleCount = sampleCount;
    }

    float32 sampleSpread = _shadowSampleSpread;
    if (ImGui::SliderFloat("Sample Spread", &sampleSpread, 1.0f, 1000.0f))
    {
      _shadowSampleSpread = sampleSpread;
    }
  }
  ImGui::Separator();
  {
    ImGui::Text("Cascaded Shadow Maps");

    float32 cascadeLambda = _cascadeLambda;
    if (ImGui::SliderFloat("Lambda", &cascadeLambda, 0.01f, 1.0f))
    {
      _cascadeLambda = cascadeLambda;
    }

    bool shouldDrawCascadeLayers = _drawCascadeLayers;
    if (ImGui::Checkbox("Draw Layers", &shouldDrawCascadeLayers))
    {
      _drawCascadeLayers = shouldDrawCascadeLayers;
    }
  }
  ImGui::Separator();
  {
    ImGui::Text("Visualize Render Pass");

    std::vector<const char *> debugRenderingItems = {"Disabled", "Shadow Depth", "Diffuse", "Normal", "Specular", "Depth", "Shadows", "Occulsion"};
    static int debugRenderingCurrentItem = 0;
    if (ImGui::Combo("Target", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size()))
    {
      _debugDisplayType = static_cast<DebugDisplayType>(debugRenderingCurrentItem);
    }

    if (_debugDisplayType == DebugDisplayType::ShadowDepth)
    {
      int shadowMapLayerToDraw = _shadowMapLayerToDraw;
      if (ImGui::SliderInt("Layer", &shadowMapLayerToDraw, 0, 3))
      {
        _shadowMapLayerToDraw = shadowMapLayerToDraw;
      }
    }
  }
}

void Renderer::initConstantBuffers(const std::shared_ptr<RenderDevice> &renderDevice)
{
  GpuBufferDesc perObjectBufferDesc;
  perObjectBufferDesc.BufferType = BufferType::Constant;
  perObjectBufferDesc.BufferUsage = BufferUsage::Dynamic;
  perObjectBufferDesc.ByteCount = sizeof(PerObjectBufferData);
  _perObjectBuffer = renderDevice->createGpuBuffer(perObjectBufferDesc);

  GpuBufferDesc perFrameBufferDesc;
  perFrameBufferDesc.BufferType = BufferType::Constant;
  perFrameBufferDesc.BufferUsage = BufferUsage::Dynamic;
  perFrameBufferDesc.ByteCount = sizeof(PerFrameBufferData);
  _perFrameBuffer = renderDevice->createGpuBuffer(perFrameBufferDesc);

  GpuBufferDesc ssaoConstantsDataDesc;
  ssaoConstantsDataDesc.BufferType = BufferType::Constant;
  ssaoConstantsDataDesc.BufferUsage = BufferUsage::Default;
  ssaoConstantsDataDesc.ByteCount = sizeof(SsaoConstantsData);
  _ssaoConstantsBuffer = renderDevice->createGpuBuffer(ssaoConstantsDataDesc);

  GpuBufferDesc shadowMapDebugDataDesc;
  shadowMapDebugDataDesc.BufferType = BufferType::Constant;
  shadowMapDebugDataDesc.BufferUsage = BufferUsage::Dynamic;
  shadowMapDebugDataDesc.ByteCount = sizeof(ShadowMapDebugData);
  _shadowMapDebugBuffer = renderDevice->createGpuBuffer(shadowMapDebugDataDesc);

  GpuBufferDesc fullscreenQuadDataDesc;
  fullscreenQuadDataDesc.BufferType = BufferType::Constant;
  fullscreenQuadDataDesc.BufferUsage = BufferUsage::Dynamic;
  fullscreenQuadDataDesc.ByteCount = sizeof(FullscreenQuadData);
  _fullscreenQuadBuffer = renderDevice->createGpuBuffer(fullscreenQuadDataDesc);
}

void Renderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                         const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                         const std::vector<std::shared_ptr<Drawable>> &opaqueDrawables,
                         const std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                         const std::vector<std::shared_ptr<Drawable>> &allDrawables,
                         const std::vector<std::shared_ptr<Light>> &lights,
                         const std::shared_ptr<Camera> &camera)
{
  // TODO: Need to improve this as we only support one direction light.
  std::shared_ptr<Light> directionalLight;
  for (const auto &light : lights)
  {
    if (light->getLightType() == LightType::Directional)
    {
      directionalLight = light;
      break;
    }
  }

  writePerFrameConstantData(camera, directionalLight, lights);

  directionalLightDepthPass(renderDevice, allDrawables, directionalLight, camera);
  gbufferPass(renderDevice, opaqueDrawables, camera);
  transparencyPass(renderDevice, transparentDrawables, camera);
  shadowPass(renderDevice);
  ssaoPass(renderDevice, camera);
  lightingPass(renderDevice, lights, camera);
  debugPass(renderDevice, aabbDrawables, camera);
}

void Renderer::initSamplers(const std::shared_ptr<RenderDevice> &renderDevice)
{
  SamplerStateDesc basicSamplerStateDesc;
  basicSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
  basicSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
  basicSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
  basicSamplerStateDesc.MipFiltering = TextureFilteringMode::Linear;
  _basicSamplerState = renderDevice->createSamplerState(basicSamplerStateDesc);

  SamplerStateDesc noMipSamplerState;
  noMipSamplerState.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
  noMipSamplerState.MinFiltering = TextureFilteringMode::None;
  noMipSamplerState.MinFiltering = TextureFilteringMode::None;
  noMipSamplerState.MipFiltering = TextureFilteringMode::None;
  _noMipSamplerState = renderDevice->createSamplerState(noMipSamplerState);

  SamplerStateDesc shadowMapSamplerStateDesc;
  shadowMapSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Border, TextureAddressMode::Border, TextureAddressMode::Border};
  shadowMapSamplerStateDesc.MinFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.MinFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.MipFiltering = TextureFilteringMode::None;
  shadowMapSamplerStateDesc.BorderColour = Colour::White;
  _shadowMapSamplerState = renderDevice->createSamplerState(shadowMapSamplerStateDesc);

  SamplerStateDesc ssaoNoiseSamplerDesc;
  ssaoNoiseSamplerDesc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
  ssaoNoiseSamplerDesc.MinFiltering = TextureFilteringMode::None;
  ssaoNoiseSamplerDesc.MinFiltering = TextureFilteringMode::None;
  ssaoNoiseSamplerDesc.MipFiltering = TextureFilteringMode::None;
  _ssaoNoiseSampler = renderDevice->createSamplerState(ssaoNoiseSamplerDesc);

  SamplerStateDesc noMipWithBorderSamplerState;
  noMipWithBorderSamplerState.AddressingMode = AddressingMode{TextureAddressMode::Border, TextureAddressMode::Border, TextureAddressMode::Border};
  noMipWithBorderSamplerState.MinFiltering = TextureFilteringMode::None;
  noMipWithBorderSamplerState.MinFiltering = TextureFilteringMode::None;
  noMipWithBorderSamplerState.MipFiltering = TextureFilteringMode::None;
  _noMipWithBorderSamplerState = renderDevice->createSamplerState(noMipWithBorderSamplerState);
}

void Renderer::initTextures(const std::shared_ptr<RenderDevice> &renderDevice)
{
  TextureDesc randomRotationsDesc;
  randomRotationsDesc.Width = RANDOM_ROTATION_TEXTURE_SIZE;
  randomRotationsDesc.Height = RANDOM_ROTATION_TEXTURE_SIZE;
  randomRotationsDesc.Usage = TextureUsage::Default;
  randomRotationsDesc.Type = TextureType::Texture2D;
  randomRotationsDesc.Format = TextureFormat::R8;
  _randomRotationsMap = renderDevice->createTexture(randomRotationsDesc);

  ubyte randomValues[RANDOM_ROTATION_TEXTURE_SIZE * RANDOM_ROTATION_TEXTURE_SIZE];
  srand(0);
  for (uint32 i = 0; i < RANDOM_ROTATION_TEXTURE_SIZE * RANDOM_ROTATION_TEXTURE_SIZE; i++)
  {
    randomValues[i] = static_cast<ubyte>((rand() / static_cast<float32>(RAND_MAX)) * 255.0f);
  }

  std::shared_ptr<ImageData> imageData(new ImageData(RANDOM_ROTATION_TEXTURE_SIZE, RANDOM_ROTATION_TEXTURE_SIZE, 1, ImageFormat::R8));
  imageData->writeData(randomValues);
  _randomRotationsMap->writeData(0, 0, imageData);

  TextureDesc ssaoNoiseTextureDesc;
  ssaoNoiseTextureDesc.Width = SSAO_NOISE_TEXTURE_SIZE;
  ssaoNoiseTextureDesc.Height = SSAO_NOISE_TEXTURE_SIZE;
  ssaoNoiseTextureDesc.Usage = TextureUsage::Default;
  ssaoNoiseTextureDesc.Type = TextureType::Texture2D;
  ssaoNoiseTextureDesc.Format = TextureFormat::RGB32F;
  _ssaoNoiseTexture = renderDevice->createTexture(ssaoNoiseTextureDesc);

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

  _ssaoNoiseTexture->writeData(0, 0, 0, SSAO_NOISE_TEXTURE_SIZE, 0, SSAO_NOISE_TEXTURE_SIZE, 0, 0, ssaoNoise.data());

  createDirectionalLightShadowDepthMap(renderDevice);
}

void Renderer::initDirectionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  TextureDesc shadowMapDesc;
  shadowMapDesc.Width = _shadowMapResolution;
  shadowMapDesc.Height = _shadowMapResolution;
  shadowMapDesc.Usage = TextureUsage::Depth;
  shadowMapDesc.Type = TextureType::Texture2DArray;
  shadowMapDesc.Format = TextureFormat::D32F;
  shadowMapDesc.Count = _cascadeCount;

  RenderTargetDesc rtDesc;
  rtDesc.DepthStencilTarget = renderDevice->createTexture(shadowMapDesc);
  rtDesc.Height = _shadowMapResolution;
  rtDesc.Width = _shadowMapResolution;

  _shadowMapRto = renderDevice->createRenderTarget(rtDesc);

  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/CascadeShadowMap.vert");

  // TODO: Change to only use Pixel shaders as it allows for a dynamic cascade count - apparently doesn't affect performance that much
  ShaderDesc gsDesc;
  gsDesc.ShaderType = ShaderType::Geometry;
  gsDesc.Source = String::foadFromFile("./Shaders/CascadeShadowMap.geom");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/Empty.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::Clockwise;

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->createShader(vsDesc);
  pipelineDesc.GS = renderDevice->createShader(gsDesc);
  pipelineDesc.FS = renderDevice->createShader(psDesc);
  pipelineDesc.BlendState = renderDevice->createBlendState(BlendStateDesc{});
  pipelineDesc.RasterizerState = renderDevice->createRasterizerState(RasterizerStateDesc{});
  pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _shadowMapPso = renderDevice->createPipelineState(pipelineDesc);
}

void Renderer::initGbufferPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/Gbuffer.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/Gbuffer.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
  shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
  shaderParams->addParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 3));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

  BlendStateDesc blendStateDesc{};

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->createShader(vsDesc);
  pipelineDesc.FS = renderDevice->createShader(psDesc);
  pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
  pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
  pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _gBufferPso = renderDevice->createPipelineState(pipelineDesc);

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
  rtDesc.ColourTargets[0] = renderDevice->createTexture(colourTexDesc);
  colourTexDesc.Format = TextureFormat::RGB16F;
  rtDesc.ColourTargets[1] = renderDevice->createTexture(colourTexDesc);
  colourTexDesc.Format = TextureFormat::RGBA8;
  rtDesc.ColourTargets[2] = renderDevice->createTexture(colourTexDesc);
  rtDesc.DepthStencilTarget = renderDevice->createTexture(depthStencilDesc);
  rtDesc.Height = _windowDims.X;
  rtDesc.Width = _windowDims.Y;

  _gBufferRto = renderDevice->createRenderTarget(rtDesc);
}

void Renderer::initTransparencyPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/Gbuffer.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/GbufferTransparency.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
  shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
  shaderParams->addParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 3));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

  BlendStateDesc blendStateDesc{};
  blendStateDesc.RTBlendState[0].BlendEnabled = true;
  blendStateDesc.RTBlendState[0].BlendAlpha = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->createShader(vsDesc);
  pipelineDesc.FS = renderDevice->createShader(psDesc);
  pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
  pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
  pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _transparencyPso = renderDevice->createPipelineState(pipelineDesc);
}

void Renderer::initShadowPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/Shadows.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
  };

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));
  shaderParams->addParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
  shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
  shaderParams->addParam(ShaderParam("ShadowMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("RandomRotationsMap", ShaderParamType::Texture, 3));

  RasterizerStateDesc rasterizerStateDesc{};
  BlendStateDesc blendStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->createShader(vsDesc);
  pipelineDesc.FS = renderDevice->createShader(psDesc);
  pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
  pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
  pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(depthStencilStateDesc);
  pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _shadowsPso = renderDevice->createPipelineState(pipelineDesc);

  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::R8;

  RenderTargetDesc rtDesc;
  rtDesc.ColourTargets[0] = renderDevice->createTexture(colourTexDesc);
  rtDesc.Height = _windowDims.X;
  rtDesc.Width = _windowDims.Y;

  _shadowsRto = renderDevice->createRenderTarget(rtDesc);
}

void Renderer::initSsaoPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/Ssao.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("SsaoConstantsBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->addParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->addParam(ShaderParam("NoiseMap", ShaderParamType::Texture, 2));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _ssaoPso = renderDevice->createPipelineState(pipelineDesc);
  }
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/SsaoBlur.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("SsaoMap", ShaderParamType::Texture, 0));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _ssaoBlurPso = renderDevice->createPipelineState(pipelineDesc);
  }
  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::R8;

  RenderTargetDesc rtDesc;
  rtDesc.ColourTargets[0] = renderDevice->createTexture(colourTexDesc);
  rtDesc.Height = _windowDims.X;
  rtDesc.Width = _windowDims.Y;

  _ssaoRto = renderDevice->createRenderTarget(rtDesc);
  _ssaoBlurRto = renderDevice->createRenderTarget(rtDesc);
}

void Renderer::initLightingPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/Lighting.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
  };

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));
  shaderParams->addParam(ShaderParam("CascadeShadowMapBuffer", ShaderParamType::ConstBuffer, 2));

  shaderParams->addParam(ShaderParam("AlbedoMap", ShaderParamType::Texture, 0));
  shaderParams->addParam(ShaderParam("DepthMap", ShaderParamType::Texture, 1));
  shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 3));
  shaderParams->addParam(ShaderParam("ShadowMap", ShaderParamType::Texture, 4));
  shaderParams->addParam(ShaderParam("OcculusionMap", ShaderParamType::Texture, 5));

  RasterizerStateDesc rasterizerStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  BlendStateDesc blendStateDesc{};

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->createShader(vsDesc);
  pipelineDesc.FS = renderDevice->createShader(psDesc);
  pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
  pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
  pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(depthStencilStateDesc);
  pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _lightingPso = renderDevice->createPipelineState(pipelineDesc);

  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::RGB8;

  RenderTargetDesc rtDesc;
  rtDesc.ColourTargets[0] = renderDevice->createTexture(colourTexDesc);
  rtDesc.Height = _windowDims.X;
  rtDesc.Width = _windowDims.Y;

  _lightingPassRto = renderDevice->createRenderTarget(rtDesc);
}

void Renderer::initDebugPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  // TODO: Combine these PSO into one.
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/FullscreenQuad.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("QuadTexture", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("CameraBuffer", ShaderParamType::ConstBuffer, 0));
    shaderParams->addParam(ShaderParam("FullscreenQuadBuffer", ShaderParamType::ConstBuffer, 1));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _gbufferDebugDrawPso = renderDevice->createPipelineState(pipelineDesc);
  }
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/DepthDebug.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("QuadTexture", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("CameraBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _depthDebugDrawPso = renderDevice->createPipelineState(pipelineDesc);
  }
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/ShadowMapDebug.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("ShadowMaps", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("ShadowMapDebugBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _shadowMapDebugPso = renderDevice->createPipelineState(pipelineDesc);
  }
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/Basic.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/Basic.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3)};

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc{};
    rasterizerStateDesc.FillMode = FillMode::WireFrame;
    rasterizerStateDesc.CullMode = CullMode::None;

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    BlendStateDesc blendStateDesc{};

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->createShader(vsDesc);
    pipelineDesc.FS = renderDevice->createShader(psDesc);
    pipelineDesc.BlendState = renderDevice->createBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = renderDevice->createRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->createDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = renderDevice->createVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;
    pipelineDesc.Topology = PrimitiveTopology::LineList;

    _drawAabbPso = renderDevice->createPipelineState(pipelineDesc);
  }
}

void Renderer::directionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                         const std::vector<std::shared_ptr<Drawable>> &drawables,
                                         const std::shared_ptr<Light> &directionalLight,
                                         const std::shared_ptr<Camera> &camera)
{
  if (_shadowResolutionChanged)
  {
    createDirectionalLightShadowDepthMap(renderDevice);
  }

  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->setPipelineState(_shadowMapPso);

  ViewportDesc viewportDesc;
  viewportDesc.Height = _shadowMapResolution;
  viewportDesc.Width = _shadowMapResolution;
  renderDevice->setViewport(viewportDesc);
  renderDevice->setRenderTarget(_shadowMapRto);
  renderDevice->clearBuffers(RTT_Depth);
  renderDevice->setConstantBuffer(0, _perObjectBuffer);
  renderDevice->setConstantBuffer(1, _perFrameBuffer);

  for (const auto &drawable : drawables)
  {
    std::shared_ptr<Material> material(drawable->getMaterial());
    drawDrawable(renderDevice, drawable, material, camera);
  }

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[0].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                           const std::vector<std::shared_ptr<Drawable>> &drawables,
                           const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  renderDevice->setViewport(viewportDesc);

  renderDevice->setPipelineState(_gBufferPso);
  renderDevice->setRenderTarget(_gBufferRto);
  renderDevice->clearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);
  renderDevice->setConstantBuffer(0, _perObjectBuffer);

  for (const auto &drawable : drawables)
  {
    std::shared_ptr<Material> material(drawable->getMaterial());
    if (material->hasDiffuseTexture())
    {
      renderDevice->setTexture(0, material->getDiffuseTexture());
      renderDevice->setSamplerState(0, _basicSamplerState);
    }
    if (material->hasNormalTexture())
    {
      renderDevice->setTexture(1, material->getNormalTexture());
      renderDevice->setSamplerState(1, _noMipSamplerState);
    }
    if (material->hasSpecularTexture())
    {
      renderDevice->setTexture(2, material->getSpecularTexture());
      renderDevice->setSamplerState(2, _noMipSamplerState);
    }

    drawDrawable(renderDevice, drawable, material, camera);
  }

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[1].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::transparencyPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                const std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                                const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->setPipelineState(_transparencyPso);
  renderDevice->setRenderTarget(_gBufferRto);
  renderDevice->setConstantBuffer(0, _perObjectBuffer);

  for (const auto &drawable : transparentDrawables)
  {
    std::shared_ptr<Material> material(drawable->getMaterial());
    if (material->hasDiffuseTexture())
    {
      renderDevice->setTexture(0, material->getDiffuseTexture());
      renderDevice->setSamplerState(0, _basicSamplerState);
    }
    if (material->hasNormalTexture())
    {
      renderDevice->setTexture(1, material->getNormalTexture());
      renderDevice->setSamplerState(1, _noMipSamplerState);
    }
    if (material->hasSpecularTexture())
    {
      renderDevice->setTexture(2, material->getSpecularTexture());
      renderDevice->setSamplerState(2, _noMipSamplerState);
    }
    if (material->hasOpacityTexture())
    {
      renderDevice->setTexture(3, material->getOpacityTexture());
      renderDevice->setSamplerState(3, _noMipSamplerState);
    }

    drawDrawable(renderDevice, drawable, material, camera);
  }

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[2].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::shadowPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->setPipelineState(_shadowsPso);
  renderDevice->setRenderTarget(_shadowsRto);
  renderDevice->setTexture(0, _gBufferRto->getDepthStencilTarget());
  renderDevice->setTexture(1, _gBufferRto->getColourTarget(1));
  renderDevice->setTexture(2, _shadowMapRto->getDepthStencilTarget());
  renderDevice->setTexture(3, _randomRotationsMap);
  renderDevice->setConstantBuffer(1, _perFrameBuffer);
  renderDevice->setSamplerState(0, _noMipSamplerState);
  renderDevice->setSamplerState(1, _noMipSamplerState);
  renderDevice->setSamplerState(2, _shadowMapSamplerState);
  renderDevice->setSamplerState(3, _noMipSamplerState);
  renderDevice->setVertexBuffer(_fsQuadVertexBuffer);
  renderDevice->draw(6, 0);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[3].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  if (_ssaoSettingsModified)
  {
    writeSsaoConstantData(renderDevice, camera);
    _ssaoSettingsModified = false;
  }

  renderDevice->setPipelineState(_ssaoPso);
  renderDevice->setRenderTarget(_ssaoRto);
  renderDevice->setTexture(0, _gBufferRto->getDepthStencilTarget());
  renderDevice->setTexture(1, _gBufferRto->getColourTarget(1));
  renderDevice->setTexture(2, _ssaoNoiseTexture);
  renderDevice->setSamplerState(0, _noMipSamplerState);
  renderDevice->setSamplerState(1, _noMipSamplerState);
  renderDevice->setSamplerState(2, _ssaoNoiseSampler);
  renderDevice->setConstantBuffer(0, _ssaoConstantsBuffer);
  renderDevice->setVertexBuffer(_fsQuadVertexBuffer);
  renderDevice->draw(6, 0);

  renderDevice->setPipelineState(_ssaoBlurPso);
  renderDevice->setRenderTarget(_ssaoBlurRto);
  renderDevice->setTexture(0, _ssaoRto->getColourTarget(0));
  renderDevice->setSamplerState(0, _noMipSamplerState);
  renderDevice->setVertexBuffer(_fsQuadVertexBuffer);
  renderDevice->draw(6, 0);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[4].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::lightingPass(std::shared_ptr<RenderDevice> renderDevice,
                            const std::vector<std::shared_ptr<Light>> &lights,
                            const std::shared_ptr<Camera> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  renderDevice->setPipelineState(_lightingPso);
  renderDevice->setRenderTarget(_lightingPassRto);
  renderDevice->setTexture(0, _gBufferRto->getColourTarget(0));
  renderDevice->setTexture(1, _gBufferRto->getDepthStencilTarget());
  renderDevice->setTexture(2, _gBufferRto->getColourTarget(1));
  renderDevice->setTexture(3, _gBufferRto->getColourTarget(2));
  renderDevice->setTexture(4, _shadowsRto->getColourTarget(0));
  renderDevice->setTexture(5, _ssaoBlurRto->getColourTarget(0));
  renderDevice->setSamplerState(0, _noMipSamplerState);
  renderDevice->setSamplerState(1, _noMipSamplerState);
  renderDevice->setSamplerState(2, _noMipSamplerState);
  renderDevice->setSamplerState(3, _noMipSamplerState);
  renderDevice->setSamplerState(4, _noMipSamplerState);
  renderDevice->setSamplerState(5, _noMipSamplerState);
  renderDevice->setConstantBuffer(1, _perFrameBuffer);

  renderDevice->setVertexBuffer(_fsQuadVertexBuffer);
  renderDevice->draw(6, 0);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[5].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::debugPass(const std::shared_ptr<RenderDevice> &renderDevice,
                         const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                         const std::shared_ptr<Camera> &camera)
{
  switch (_debugDisplayType)
  {
  case DebugDisplayType::Disabled:
  {
    drawDebugRenderTarget(renderDevice, _lightingPassRto->getColourTarget(0), camera);
    break;
  }
  case DebugDisplayType::ShadowDepth:
  {
    drawDebugRenderTarget(renderDevice, _shadowMapRto->getDepthStencilTarget(), camera);
    break;
  }
  case DebugDisplayType::Diffuse:
  {
    drawDebugRenderTarget(renderDevice, _gBufferRto->getColourTarget(0), camera);
    break;
  }
  case DebugDisplayType::Normal:
  {
    drawDebugRenderTarget(renderDevice, _gBufferRto->getColourTarget(1), camera);
    break;
  }
  case DebugDisplayType::Specular:
  {
    drawDebugRenderTarget(renderDevice, _gBufferRto->getColourTarget(2), camera);
    break;
  }
  case DebugDisplayType::Depth:
  {
    drawDebugRenderTarget(renderDevice, _gBufferRto->getDepthStencilTarget(), camera);
    break;
  }
  case DebugDisplayType::Shadows:
  {
    drawDebugRenderTarget(renderDevice, _shadowsRto->getColourTarget(0), camera, true);
    break;
  }
  case DebugDisplayType::Occulsion:
  {
    drawDebugRenderTarget(renderDevice, _ssaoBlurRto->getColourTarget(0), camera, true);
    break;
  }
  }

  drawAabb(renderDevice, aabbDrawables, camera);
}

void Renderer::drawDrawable(const std::shared_ptr<RenderDevice> &renderDevice,
                            const std::shared_ptr<Drawable> &drawable,
                            const std::shared_ptr<Material> &material,
                            const std::shared_ptr<Camera> &camera)
{
  writePerObjectConstantData(drawable, material, camera);

  std::shared_ptr<StaticMesh> mesh = drawable->getMesh();
  renderDevice->setVertexBuffer(mesh->getVertexData(renderDevice));

  if (mesh->isIndexed())
  {
    auto indexCount = mesh->getIndexCount();
    renderDevice->setIndexBuffer(mesh->getIndexData(renderDevice));
    renderDevice->drawIndexed(indexCount, 0, 0);
  }
  else
  {
    auto vertexCount = mesh->getVertexCount();
    renderDevice->draw(vertexCount, 0);
  }
}

void Renderer::drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                        const std::shared_ptr<Camera> &camera)
{
  renderDevice->setPipelineState(_drawAabbPso);
  for (auto drawable : aabbDrawables)
  {
    auto &aabb = drawable->getAabb();

    PerObjectBufferData objectBufferData;
    objectBufferData.Model = Matrix4::Translation(drawable->getPosition()) * Matrix4::Scaling(aabb.getExtents());
    objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
    objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
    _perObjectBuffer->writeData(0, sizeof(PerObjectBufferData), &objectBufferData, AccessType::WriteOnlyDiscard);

    renderDevice->setConstantBuffer(0, _perObjectBuffer);
    renderDevice->setVertexBuffer(_aabbVertexBuffer);
    renderDevice->draw(AabbCoords.size(), 0);
  }
}

void Renderer::drawDebugRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                                     std::shared_ptr<Texture> renderTarget,
                                     const std::shared_ptr<Camera> &camera,
                                     bool singleChannelImage)
{
  ShadowMapDebugData shadowMapDebugData{};
  shadowMapDebugData.FarClip = camera->getFar();
  shadowMapDebugData.NearClip = camera->getNear();
  shadowMapDebugData.Layer = _shadowMapLayerToDraw;
  _shadowMapDebugBuffer->writeData(0, sizeof(ShadowMapDebugData), &shadowMapDebugData, AccessType::WriteOnlyDiscard);

  FullscreenQuadData fullscreenQuadData{};
  fullscreenQuadData.SingleChannel = singleChannelImage;
  _fullscreenQuadBuffer->writeData(0, sizeof(FullscreenQuadData), &fullscreenQuadData, AccessType::WriteOnlyDiscard);
  renderDevice->setConstantBuffer(1, _fullscreenQuadBuffer);

  if (renderTarget->getDesc().Usage == TextureUsage::RenderTarget)
  {
    renderDevice->setPipelineState(_gbufferDebugDrawPso);
  }
  else if (renderTarget->getDesc().Usage == TextureUsage::Depth)
  {
    if (renderTarget->getTextureType() == TextureType::Texture2DArray)
    {
      renderDevice->setPipelineState(_shadowMapDebugPso);
      renderDevice->setConstantBuffer(0, _shadowMapDebugBuffer);
    }
    else if (renderTarget->getTextureType() == TextureType::Texture2D)
    {
      renderDevice->setPipelineState(_depthDebugDrawPso);
      renderDevice->setConstantBuffer(0, _shadowMapDebugBuffer);
    }
    else
    {
      return;
    }
  }
  else
  {
    return;
  }

  renderDevice->setRenderTarget(nullptr);
  renderDevice->clearBuffers(RTT_Colour | RTT_Depth);
  renderDevice->setTexture(0, renderTarget);
  renderDevice->setSamplerState(0, _noMipWithBorderSamplerState);
  renderDevice->setVertexBuffer(_fsQuadVertexBuffer);
  renderDevice->draw(6, 0);
}

std::vector<Matrix4> Renderer::calculateCameraCascadeProjections(const std::shared_ptr<Camera> &camera) const
{
  Radian fov = camera->getFov();
  float32 aspect = camera->getAspectRatio();
  float32 nearPlane = camera->getNear();
  float32 farPlane = camera->getFar();

  std::vector<float32> cascadeLevels(calculateCascadeLevels(nearPlane, farPlane));

  std::vector<Matrix4> projections;
  projections.push_back(Matrix4::Perspective(fov, aspect, nearPlane, cascadeLevels[0]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[0], cascadeLevels[1]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[1], cascadeLevels[2]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[2], farPlane));
  return projections;
}

std::vector<float32> Renderer::calculateCascadeLevels(float32 nearClip, float32 farClip) const
{
  float32 clipRange = farClip - nearClip;

  float32 minZ = nearClip + _minCascadeDistance * clipRange;
  float32 maxZ = nearClip + _maxCascadeDistance * clipRange;

  float32 range = maxZ - minZ;
  float32 ratio = maxZ / minZ;

  std::vector<float32> cascadeSplits;
  for (uint32 i = 0; i < _cascadeCount; ++i)
  {
    float32 p = (i + 1) / static_cast<float32>(_cascadeCount);
    float32 log = minZ * std::pow(ratio, p);
    float32 uniform = minZ + range * p;
    float32 d = _cascadeLambda * (log - uniform) + uniform;
    cascadeSplits.push_back(d);
  }

  return cascadeSplits;
}

std::vector<Matrix4> Renderer::calculateCascadeLightTransforms(const std::shared_ptr<Camera> &camera, const std::shared_ptr<Light> &directionalLight) const
{
  std::vector<Matrix4> results;
  std::vector<Matrix4> projections = calculateCameraCascadeProjections(camera);
  for (uint32 i = 0; i < _cascadeCount; i++)
  {
    auto frustrumCorners = calculateFrustrumCorners(camera->getView(), projections[i]);
    Vector3 frustrumCenter = calculateFrustrumCenter(frustrumCorners);
    const float32 radius = calculateCascadeRadius(frustrumCorners, frustrumCenter);

    Vector3 maxExtents(radius, radius, radius);
    Vector3 minExtents = -maxExtents;
    Vector3 cascadeExtents = maxExtents - minExtents;

    Vector3 lightDirection = -directionalLight->getDirection();
    Vector3 shadowCameraPos = frustrumCenter + lightDirection * -minExtents.Z;
    Matrix4 shadowCameraView = Matrix4::LookAt(shadowCameraPos, frustrumCenter, Vector3::Up);

    Matrix4 shadowCameraProj = Matrix4::Orthographic(-radius, radius, -radius, radius, -cascadeExtents.Z, cascadeExtents.Z);
    Matrix4 shadowMatrix = shadowCameraProj * shadowCameraView;
    Vector4 shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    shadowOrigin = shadowOrigin * _shadowMapResolution / 2.0f;

    Vector4 roundedOrigin = Math::RoundToEven(shadowOrigin);
    Vector4 roundedOffset = roundedOrigin - shadowOrigin;
    roundedOffset = roundedOffset * (2.0f / _shadowMapResolution);
    roundedOffset.Z = 0.0f;
    roundedOffset.W = 0.0f;

    shadowCameraProj[3] += roundedOffset;

    results.push_back(shadowCameraProj * shadowCameraView);
  }
  return results;
}

void Renderer::createDirectionalLightShadowDepthMap(const std::shared_ptr<RenderDevice> &renderDevice)
{
  TextureDesc shadowMapDesc;
  shadowMapDesc.Width = _shadowMapResolution;
  shadowMapDesc.Height = _shadowMapResolution;
  shadowMapDesc.Usage = TextureUsage::Depth;
  shadowMapDesc.Type = TextureType::Texture2DArray;
  shadowMapDesc.Format = TextureFormat::D32F;
  shadowMapDesc.Count = 4;

  RenderTargetDesc rtDesc;
  rtDesc.DepthStencilTarget = renderDevice->createTexture(shadowMapDesc);
  rtDesc.Height = _shadowMapResolution;
  rtDesc.Width = _shadowMapResolution;

  _shadowMapRto = renderDevice->createRenderTarget(rtDesc);
  _shadowResolutionChanged = false;
}

void Renderer::writePerObjectConstantData(const std::shared_ptr<Drawable> &drawable,
                                          const std::shared_ptr<Material> &material,
                                          const std::shared_ptr<Camera> &camera) const
{
  PerObjectBufferData perObjectBufferData{};
  perObjectBufferData.Model = drawable->getMatrix();
  perObjectBufferData.ModelView = camera->getView() * perObjectBufferData.Model;
  perObjectBufferData.ModelViewProjection = camera->getProj() * perObjectBufferData.ModelView;
  perObjectBufferData.AmbientColour = material->getAmbientColour();
  perObjectBufferData.DiffuseColour = material->getDiffuseColour();
  perObjectBufferData.SpecularColour = material->getSpecularColour();
  perObjectBufferData.DiffuseEnabled = material->hasDiffuseTexture();
  perObjectBufferData.NormalEnabled = material->hasNormalTexture();
  perObjectBufferData.OpacityEnabled = material->hasOpacityTexture();
  perObjectBufferData.SpecularEnabled = material->hasSpecularTexture();
  perObjectBufferData.SpecularExponent = material->getSpecularExponent();

  _perObjectBuffer->writeData(0, sizeof(PerObjectBufferData), &perObjectBufferData, AccessType::WriteOnlyDiscard);
}

void Renderer::writePerFrameConstantData(const std::shared_ptr<Camera> &camera,
                                         const std::shared_ptr<Light> &directionalLight,
                                         const std::vector<std::shared_ptr<Light>> &lights) const
{
  // Creating this on the heap as I was exceeding stack size.
  PerFrameBufferData *perFrameBufferData = new PerFrameBufferData();
  perFrameBufferData->AmbientColour = _ambientColour.ToVec3();
  perFrameBufferData->AmbientIntensity = _ambientIntensity;
  perFrameBufferData->CascadeLayerCount = _cascadeCount;

  std::vector<Matrix4> cascadeLightTransforms(calculateCascadeLightTransforms(camera, directionalLight));
  std::vector<float32> cascadeLevels(calculateCascadeLevels(camera->getNear(), camera->getFar()));
  for (uint32 i = 0; i < _cascadeCount; i++)
  {
    perFrameBufferData->CascadeLightTransforms[i] = cascadeLightTransforms[i];
    perFrameBufferData->CascadePlaneDistances[i].X = cascadeLevels[i];
  }
  perFrameBufferData->DrawCascadeLayers = _drawCascadeLayers;
  perFrameBufferData->FarPlane = camera->getFar();
  perFrameBufferData->LightColour = directionalLight->getColour().ToVec3();
  perFrameBufferData->LightDirection = directionalLight->getDirection();
  perFrameBufferData->LightIntensity = directionalLight->getIntensity();
  perFrameBufferData->ShadowSampleCount = _shadowSampleCount;
  perFrameBufferData->ShadowSampleSpread = _shadowSampleSpread;
  perFrameBufferData->SsaoEnabled = _ssaoEnabled;
  perFrameBufferData->View = camera->getView();
  perFrameBufferData->Proj = camera->getProj();
  perFrameBufferData->ProjInv = perFrameBufferData->Proj.Inverse();
  perFrameBufferData->ProjViewInv = (perFrameBufferData->Proj * perFrameBufferData->View).Inverse();
  perFrameBufferData->ViewPosition = camera->getParentTransform().getPosition();

  std::vector<LightData> lightDataArray;
  for (uint32 i = 0; i < lights.size(); i++)
  {
    const auto &light = lights[i];
    // TODO: Need to improve this as we only support one direction light.
    if (light->getLightType() != LightType::Directional)
    {
      LightData lightData;
      lightData.Colour = light->getColour().ToVec3();
      lightData.Intensity = light->getIntensity();
      lightData.Position = light->getPosition();
      lightData.Radius = light->getRadius();
      lightDataArray.push_back(lightData);
    }
  }
  std::copy(lightDataArray.begin(), lightDataArray.end(), perFrameBufferData->Lights);
  perFrameBufferData->LightCount = lightDataArray.size();

  _perFrameBuffer->writeData(0, sizeof(PerFrameBufferData), perFrameBufferData, AccessType::WriteOnlyDiscard);
  delete perFrameBufferData;
}

void Renderer::writeSsaoConstantData(const std::shared_ptr<RenderDevice> &renderDevice,
                                     const std::shared_ptr<Camera> &camera) const
{
  std::uniform_real_distribution<float32> randomFloats(0.0, 1.0);
  std::default_random_engine generator;
  std::vector<Vector3> ssaoKernel;

  for (uint32 i = 0; i < SSAO_MAX_KERNAL_SIZE; ++i)
  {
    Vector3 sample(randomFloats(generator) * 2.0f - 1.0f,
                   randomFloats(generator) * 2.0f - 1.0f,
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
  ssaoConstantsData.Bias = _ssaoBias;
  ssaoConstantsData.Radius = _ssaoRadius;
  ssaoConstantsData.KernelSize = _ssaoSamples;
  for (uint32 i = 0; i < SSAO_MAX_KERNAL_SIZE; ++i)
  {
    ssaoConstantsData.NoiseSamples[i] = ssaoKernel[i];
  }
  _ssaoConstantsBuffer->writeData(0, sizeof(SsaoConstantsData), &ssaoConstantsData, AccessType::WriteOnlyDiscard);
}