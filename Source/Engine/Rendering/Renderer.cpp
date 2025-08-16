// Core headers
#include "Renderer.h"
#include "../Core/ComponentBase.inl"
#include <random> // for mt19937 and uniform distributions
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>

// Global deterministic RNG for SSAO noise and kernel
static std::mt19937 g_ssaoGenerator(0);

#include "../Core/Maths.h"
#include "../RenderApi/BlendState.hpp"
#include "../RenderApi/DepthStencilState.hpp"
#include "../RenderApi/Shader.hpp"
#include "../RenderApi/GraphicsPipelineState.hpp"
// Renderer migrated to Framebuffer API; legacy RenderTarget removed
#include "../RenderApi/RasterizerState.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../RenderApi/Texture.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../RenderApi/VertexLayout.hpp"
#include "../RenderApi/CommandBuffer.hpp"
#include "../UI/ImGui/imgui.h"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "CameraComponent.h"
#include "../RenderApi/RenderPass.hpp"
#include "../RenderApi/Framebuffer.hpp"
#include "DrawableComponent.h"
#include "LightComponent.h"
#include "Material.h"
#include "StaticMesh.h"
#include "RenderQueue.h"
#include "../RenderApi/Queue.hpp"

const static uint32 RANDOM_ROTATION_TEXTURE_SIZE = 64;
const static uint32 SSAO_NOISE_TEXTURE_SIZE = 4;
const static uint32 SSAO_MAX_KERNAL_SIZE = 512;
const static uint32 MAX_LIGHTS = 1024;
const static uint32 MAX_CASCADE_LAYERS = 4;
const static uint32 MAX_POINT_LIGHT_SHADOW_CASTERS = 8;

struct SsaoConstantsData
{
  Vector4 NoiseSamples[SSAO_MAX_KERNAL_SIZE];
  uint32 KernelSize;
  float32 Radius;
  float32 Bias;
  float32 Intensity;
};

struct PerObjectBufferData
{
  Matrix4 Model;
  Matrix4 ModelView;
  Matrix4 ModelViewProjection;
  Colour DiffuseColour;
  int32 DiffuseEnabled = 0;
  int32 NormalEnabled = 0;
  int32 MetalnessEnabled = 0;
  int32 RoughnessEnabled = 0;
  int32 OcclusionEnabled = 0;
  int32 OpacityEnabled = 0;
  float32 Metalness = 0.0f;
  float32 Roughness = 0.0f;
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
  Vector4 CascadePlaneDistances[MAX_CASCADE_LAYERS]; // std140 layout requires 16-byte alignment per element
  // --------- Alignment ----------
  uint32 LightCount;
  float32 Exposure;
  int32 ToneMappingEnabled;
  float32 BloomStrength;
  // --------- Alignment ----------
  float32 BloomThreshold;
  float32 MaxPointLightShadowCasters; // Maximum number of point lights that cast shadows
  float32 _padding[2];                // Ensure 16-byte alignment at end
};

struct BloomBuffer
{
  Vector2 SourceResolution;
  float32 FilterRadius;
  float32 _padding; // Ensure 16-byte alignment for std140 layout
};

struct PointLightBufferData
{
  Vector3 Position;
  float32 FarPlane;
  Matrix4 shadowMatrices[6];
  int32 LightIndex;     // Index of the light in the global light array
  float32 __Padding[3]; // Padding for alignment
};

struct TexturedQuadBuffer
{
  int32 PerspectiveDepth;
  int32 OrthographicDepth;
  int32 SingleChannel;
  int32 TextureArray;
  float32 NearClip;
  float32 FarClip;
  uint32 ArraySlice;
  int32 CubeArray;
};

struct FullscreenQuadVertex
{
  Vector2 Position;
  Vector2 TexCoord;

  FullscreenQuadVertex(const Vector2 &position, const Vector2 &texCoord) : Position(position), TexCoord(texCoord)
  {
  }
};

std::vector<Vector3>
    AabbCoords = {
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

// No adapter needed; use owned Framebuffer + textures per pass

// Helper: submit a recorded command buffer via the device graphics queue
static inline void SubmitRecorded(std::shared_ptr<RenderDevice> device, const std::unique_ptr<ICommandBuffer> &cmd)
{
  if (!cmd)
    return;
  auto q = device ? device->getGraphicsQueue() : nullptr;
  Assert::throwIfFalse(q != nullptr, "RenderDevice must provide a graphics queue");
  std::vector<std::shared_ptr<ICommandBuffer>> cbs;
  cbs.emplace_back(cmd.get(), [](ICommandBuffer *) {});
  SubmitInfo info{};
  info.commandBuffers = &cbs;
  q->submit(info);
}

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
                                                 _ambientColour(Colour(44, 62, 80)), // Modern neutral blue-gray (sRGB: 0.173, 0.243, 0.314)
                                                 _ambientIntensity(0.15f),
                                                 _ssaoSamples(64),
                                                 _ssaoBias(0.02f),
                                                 _ssaoRadius(0.75f),
                                                 _ssaoIntensity(2.0f),
                                                 _ssaoEnabled(true),
                                                 _drawCascadeLayers(false),
                                                 _shadowResolutionChanged(true),
                                                 _shadowMapResolution(2048),
                                                 _pointLightShadowMapResolution(1024),
                                                 _cascadeCount(4),
                                                 _shadowSampleCount(16),
                                                 _shadowSampleSpread(800.0f),
                                                 _minCascadeDistance(0.0f),
                                                 _maxCascadeDistance(1.0f),
                                                 _cascadeLambda(0.4f),
                                                 _toneMappingEnabled(true),
                                                 _bloomEnabled(true),
                                                 _exposure(1.0f),
                                                 _bloomStrength(0.15f),
                                                 _bloomFilter(0.016f),
                                                 _bloomThreshold(1.0f),
                                                 _debugDisplayType(DebugDisplayType::Disabled),
                                                 _shadowMapLayerToDraw(0),
                                                 _pointLightCubeMapToDraw(0),
                                                 _ssaoSettingsModified(true)

{
  _renderPassTimings.push_back({0, "Shadow Depth"});
  _renderPassTimings.push_back({0, "G-Buffer"});
  _renderPassTimings.push_back({0, "Transparency"});
  _renderPassTimings.push_back({0, "Shadow Merge"});
  _renderPassTimings.push_back({0, "SSAO"});
  _renderPassTimings.push_back({0, "Lighting"});
  _renderPassTimings.push_back({0, "Bloom Blur"});
  _renderPassTimings.push_back({0, "Tone Mapping"});

  // Initialize render queues
  _opaqueQueue = std::make_unique<RenderQueue>(RenderListType::Opaque);
  _transparentQueue = std::make_unique<RenderQueue>(RenderListType::Transparent);

  // Initialize shadow culling system
  _shadowFrustum = std::make_unique<ShadowFrustum>();
  _shadowQueue = std::make_unique<RenderQueue>(RenderListType::Shadow);

  // Initialize point light culling system
  _pointLightCuller = std::make_unique<PointLightCuller>();

  // Configure default point light culling settings
  _pointLightCullingSettings.enableSphereCulling = true;
  _pointLightCullingSettings.enableFaceCulling = true;
  _pointLightCullingSettings.enableDistanceLOD = true;
  _pointLightCullingSettings.maxShadowDistance = 200.0f;
  _pointLightCullingSettings.minObjectSize = 0.1f;
  _pointLightCullingSettings.faceCullingExpansion = 1.05f; // 5% expansion to avoid edge artifacts
  _pointLightCuller->setCullingSettings(_pointLightCullingSettings);
}

bool Renderer::init(const std::shared_ptr<RenderDevice> &renderDevice)
{
  try
  {
    // Create fullscreen quad vertex buffer
    VertexBufferDesc vtxBuffDesc;
    vtxBuffDesc.BufferUsage = BufferUsage::Default;
    vtxBuffDesc.VertexCount = FullscreenQuadVertices.size();
    vtxBuffDesc.VertexSizeBytes = sizeof(FullscreenQuadVertex);
    _fsQuadVertexBuffer = renderDevice->createVertexBuffer(vtxBuffDesc);
    _fsQuadVertexBuffer->writeData(0, sizeof(FullscreenQuadVertex) * FullscreenQuadVertices.size(), FullscreenQuadVertices.data(), AccessType::WriteOnlyDiscardRange);

    // Create AABB vertex buffer
    VertexBufferDesc aabbVertexBuffDesc;
    aabbVertexBuffDesc.BufferUsage = BufferUsage::Default;
    aabbVertexBuffDesc.VertexCount = AabbCoords.size();
    aabbVertexBuffDesc.VertexSizeBytes = sizeof(Vector3) * AabbCoords.size();
    _aabbVertexBuffer = renderDevice->createVertexBuffer(aabbVertexBuffDesc);
    _aabbVertexBuffer->writeData(0, sizeof(Vector3) * AabbCoords.size(), AabbCoords.data(), AccessType::WriteOnlyDiscardRange);

    initSamplers(renderDevice);
    initTextures(renderDevice);
    initConstantBuffers(renderDevice);
    initPointLightDepthPass(renderDevice);
    initDirectionalLightDepthPass(renderDevice);
    initGbufferPass(renderDevice);
    initTransparencyPass(renderDevice);
    initSsaoPass(renderDevice);
    initLightingPass(renderDevice);
    initBloomDownSamplePass(renderDevice);
    initBloomUpSamplePass(renderDevice);
    initToneMappingPass(renderDevice);
    initDebugPass(renderDevice);

    // Initialize resource sets after all resources are created
    initResourceSets(renderDevice);

    // Initialize command buffers after all other resources are ready
    initCommandBuffers(renderDevice);
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
  if (ImGui::CollapsingHeader("Renderer Settings", ImGuiTreeNodeFlags_DefaultOpen))
  {
    ImGui::Text("Ambient Lighting");

    float32 rawCol[]{_ambientColour[0], _ambientColour[1], _ambientColour[2]};
    if (ImGui::ColorEdit3("Colour", rawCol))
    {
      auto toByte = [](float32 v) -> uint8
      {
        v = std::clamp(v, 0.0f, 1.0f);
        return static_cast<uint8>(v * 255.0f + 0.5f);
      };
      _ambientColour = Colour(toByte(rawCol[0]), toByte(rawCol[1]), toByte(rawCol[2]));
    }

    float32 ambientIntensity = _ambientIntensity;
    if (ImGui::SliderFloat("Intensity", &ambientIntensity, 0.0f, 1.0f))
    {
      _ambientIntensity = ambientIntensity;
    }
    ImGui::Separator();
    ImGui::Text("Ambient Occlusion");

    // Ambient Occlusion controls
    float32 aoRadius = _ssaoRadius;
    if (ImGui::SliderFloat("AO Radius", &aoRadius, 0.1f, 2.0f))
    {
      _ssaoRadius = aoRadius;
      _ssaoSettingsModified = true;
    }

    float32 aoBias = _ssaoBias;
    if (ImGui::SliderFloat("AO Bias", &aoBias, 0.0f, 0.1f))
    {
      _ssaoBias = aoBias;
      _ssaoSettingsModified = true;
    }

    float32 aoStrength = _ssaoIntensity;
    if (ImGui::SliderFloat("AO Strength", &aoStrength, 0.1f, 5.0f))
    {
      _ssaoIntensity = aoStrength;
      _ssaoSettingsModified = true;
    }

    int32 sampleCount = _ssaoSamples;
    if (ImGui::SliderInt("Sample Count", &sampleCount, 8, 512))
    {
      _ssaoSamples = sampleCount;
      _ssaoSettingsModified = true;
    }

    bool aoEnabled = _ssaoEnabled;
    if (ImGui::Checkbox("AO On", &aoEnabled))
    {
      _ssaoEnabled = aoEnabled;
    }

    // AO Quality Presets
    ImGui::Spacing();
    ImGui::Text("AO Presets:");
    ImGui::SameLine();
    if (ImGui::Button("Low"))
    {
      _ssaoSamples = 16;
      _ssaoIntensity = 1.0f;
      _ssaoSettingsModified = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Medium"))
    {
      _ssaoSamples = 64;
      _ssaoIntensity = 2.0f;
      _ssaoSettingsModified = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("High"))
    {
      _ssaoSamples = 128;
      _ssaoIntensity = 3.0f;
      _ssaoSettingsModified = true;
    }

    ImGui::Separator();
    ImGui::Text("Shadow Quality");

    // Shadow Quality Sliders
    int shadowMapResolution = _shadowMapResolution;
    if (ImGui::SliderInt("Shadow Resolution", &shadowMapResolution, 512, 4096))
    {
      _shadowMapResolution = shadowMapResolution;
      _shadowResolutionChanged = true;
    }

    int shadowSamplesUI = _shadowSampleCount;
    if (ImGui::SliderInt("Shadow Samples", &shadowSamplesUI, 4, 32))
    {
      _shadowSampleCount = shadowSamplesUI;
      _shadowResolutionChanged = true;
    }

    float32 sampleSpread = _shadowSampleSpread;
    if (ImGui::SliderFloat("Shadow Softness", &sampleSpread, 100.0f, 2000.0f))
    {
      _shadowSampleSpread = sampleSpread;
      _shadowResolutionChanged = true;
    }

    // Shadow Quality Presets
    ImGui::Spacing();
    ImGui::Text("Quality Presets:");
    ImGui::SameLine();
    if (ImGui::Button("Low##ShadowPreset"))
    {
      _shadowMapResolution = 1024;
      _shadowSampleCount = 8;
      _shadowSampleSpread = 600.0f;
      _cascadeCount = 3;
      _shadowResolutionChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Balanced##ShadowPreset"))
    {
      _shadowMapResolution = 2048;
      _shadowSampleCount = 16;
      _shadowSampleSpread = 800.0f;
      _cascadeCount = 4;
      _shadowResolutionChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("High##ShadowPreset"))
    {
      _shadowMapResolution = 4096;
      _shadowSampleCount = 24;
      _shadowSampleSpread = 1000.0f;
      _cascadeCount = 4;
      _shadowResolutionChanged = true;
    }

    ImGui::Separator();
    ImGui::Text("Cascaded Shadow Maps");

    int cascadeCount = _cascadeCount;
    // Clamp maximum levels to MAX_CASCADE_LAYERS
    if (ImGui::SliderInt("Cascade Levels", &cascadeCount, 2, static_cast<int>(MAX_CASCADE_LAYERS)))
    {
      _cascadeCount = cascadeCount;
      _shadowResolutionChanged = true; // Trigger shadow map rebuild
    }

    float32 cascadeLambda = _cascadeLambda;
    if (ImGui::SliderFloat("Cascade Distribution", &cascadeLambda, 0.1f, 0.9f))
    {
      _cascadeLambda = cascadeLambda;
    }

    float32 shadowDistance = _maxCascadeDistance;
    if (ImGui::SliderFloat("Shadow Distance", &shadowDistance, 0.3f, 1.0f))
    {
      _maxCascadeDistance = shadowDistance;
    }

    bool shouldDrawCascadeLayers = _drawCascadeLayers;
    if (ImGui::Checkbox("Show Cascade Layers", &shouldDrawCascadeLayers))
    {
      _drawCascadeLayers = shouldDrawCascadeLayers;
    }
    ImGui::Separator();
    ImGui::Text("HDR");

    float32 exposure = _exposure;
    if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 10.0f))
    {
      _exposure = exposure;
    }
    float32 bloomStrength = _bloomStrength;
    if (ImGui::SliderFloat("Bloom Strength", &bloomStrength, 0.0f, 1.0f))
    {
      _bloomStrength = bloomStrength;
    }

    float32 bloomThreshold = _bloomThreshold;
    if (ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.5f, 3.0f))
    {
      _bloomThreshold = bloomThreshold;
    }

    bool toneMappingEnabled = _toneMappingEnabled;
    if (ImGui::Checkbox("Tone Mapping Enabled", &toneMappingEnabled))
    {
      _toneMappingEnabled = toneMappingEnabled;
    }
  }

  if (ImGui::CollapsingHeader("Visualize Render Pass"))
  {
    std::vector<const char *> debugRenderingItems = {"Disabled", "Shadow Depth", "Albedo", "Normal", "MetalRoughness", "Depth", "Point Light Shadows", "Lighting", "Occulsion"};
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
    else if (_debugDisplayType == DebugDisplayType::Shadows)
    {
      int pointLightCubeMapToDraw = _pointLightCubeMapToDraw;
      if (ImGui::SliderInt("Point Light Index", &pointLightCubeMapToDraw, 0, 7))
      {
        _pointLightCubeMapToDraw = pointLightCubeMapToDraw;
      }
    }
  }

  // Point Light Culling Settings
  if (ImGui::CollapsingHeader("Point Light Culling"))
  {
    auto cullingSettings = _pointLightCuller->getCullingSettings();
    bool settingsChanged = false;

    if (ImGui::Checkbox("Enable Sphere Culling", &cullingSettings.enableSphereCulling))
    {
      settingsChanged = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Cull objects outside the point light's radius");
    }

    if (ImGui::Checkbox("Enable Face Culling", &cullingSettings.enableFaceCulling))
    {
      settingsChanged = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Cull objects not visible from each cubemap face");
    }

    if (ImGui::Checkbox("Enable Distance LOD", &cullingSettings.enableDistanceLOD))
    {
      settingsChanged = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Skip very distant objects from shadow casting");
    }

    if (ImGui::SliderFloat("Max Shadow Distance", &cullingSettings.maxShadowDistance, 50.0f, 500.0f))
    {
      settingsChanged = true;
    }

    if (ImGui::SliderFloat("Min Object Size", &cullingSettings.minObjectSize, 0.01f, 5.0f))
    {
      settingsChanged = true;
    }

    if (ImGui::SliderFloat("Face Culling Expansion", &cullingSettings.faceCullingExpansion, 1.0f, 2.0f))
    {
      settingsChanged = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Expand frustums to reduce edge artifacts (1.0 = no expansion)");
    }

    if (settingsChanged)
    {
      _pointLightCuller->setCullingSettings(cullingSettings);
    }

    // Culling Performance Presets
    ImGui::Spacing();
    ImGui::Text("Performance Presets:");
    ImGui::SameLine();
    if (ImGui::Button("Conservative"))
    {
      cullingSettings.enableSphereCulling = true;
      cullingSettings.enableFaceCulling = false;
      cullingSettings.enableDistanceLOD = true;
      cullingSettings.maxShadowDistance = 100.0f;
      cullingSettings.minObjectSize = 0.5f;
      _pointLightCuller->setCullingSettings(cullingSettings);
    }
    ImGui::SameLine();
    if (ImGui::Button("Balanced"))
    {
      cullingSettings.enableSphereCulling = true;
      cullingSettings.enableFaceCulling = true;
      cullingSettings.enableDistanceLOD = true;
      cullingSettings.maxShadowDistance = 200.0f;
      cullingSettings.minObjectSize = 0.1f;
      cullingSettings.faceCullingExpansion = 1.1f;
      _pointLightCuller->setCullingSettings(cullingSettings);
    }
    ImGui::SameLine();
    if (ImGui::Button("Aggressive"))
    {
      cullingSettings.enableSphereCulling = true;
      cullingSettings.enableFaceCulling = true;
      cullingSettings.enableDistanceLOD = true;
      cullingSettings.maxShadowDistance = 150.0f;
      cullingSettings.minObjectSize = 0.2f;
      cullingSettings.faceCullingExpansion = 1.05f;
      _pointLightCuller->setCullingSettings(cullingSettings);
    }
  }
}

std::unique_ptr<IResourceSet> Renderer::createMaterialResourceSet(const std::shared_ptr<RenderDevice> &renderDevice,
                                                                  const std::shared_ptr<Material> &material)
{
  auto materialResourceSet = renderDevice->createResourceSet(_materialLayout);

  // Add material textures (with fallback defaults)
  materialResourceSet->addTexture(0, material->hasDiffuseTexture() ? material->getDiffuseTexture() : getDefaultWhiteTexture());
  materialResourceSet->addTexture(1, material->hasNormalTexture() ? material->getNormalTexture() : getDefaultNormalTexture());
  materialResourceSet->addTexture(2, material->hasMetallicTexture() ? material->getMetallicTexture() : getDefaultWhiteTexture());
  materialResourceSet->addTexture(3, material->hasRoughnessTexture() ? material->getRoughnessTexture() : getDefaultWhiteTexture());
  materialResourceSet->addTexture(4, material->hasOcclusionTexture() ? material->getOcclusionTexture() : getDefaultWhiteTexture());
  materialResourceSet->addTexture(5, material->hasOpacityTexture() ? material->getOpacityTexture() : getDefaultWhiteTexture());

  // Add samplers for each texture slot
  materialResourceSet->addSampler(0, _basicSamplerState); // DiffuseMap sampler
  materialResourceSet->addSampler(1, _basicSamplerState); // NormalMap sampler
  materialResourceSet->addSampler(2, _basicSamplerState); // MetallicMap sampler
  materialResourceSet->addSampler(3, _basicSamplerState); // RoughnessMap sampler
  materialResourceSet->addSampler(4, _basicSamplerState); // OcclusionMap sampler
  materialResourceSet->addSampler(5, _noMipSamplerState); // OpacityMap sampler

  materialResourceSet->build(renderDevice);
  return materialResourceSet;
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

  GpuBufferDesc fullscreenQuadDataDesc;
  fullscreenQuadDataDesc.BufferType = BufferType::Constant;
  fullscreenQuadDataDesc.BufferUsage = BufferUsage::Dynamic;
  fullscreenQuadDataDesc.ByteCount = sizeof(TexturedQuadBuffer);
  _fullscreenQuadBuffer = renderDevice->createGpuBuffer(fullscreenQuadDataDesc);

  GpuBufferDesc bloomBufferDesc;
  bloomBufferDesc.BufferType = BufferType::Constant;
  bloomBufferDesc.BufferUsage = BufferUsage::Dynamic;
  bloomBufferDesc.ByteCount = sizeof(BloomBuffer);
  _bloomBuffer = renderDevice->createGpuBuffer(bloomBufferDesc);

  GpuBufferDesc pointLightBufferDesc;
  pointLightBufferDesc.BufferType = BufferType::Constant;
  pointLightBufferDesc.BufferUsage = BufferUsage::Dynamic;
  pointLightBufferDesc.ByteCount = sizeof(PointLightBufferData);
  _pointLightBuffer = renderDevice->createGpuBuffer(pointLightBufferDesc);
}

void Renderer::initResourceSets(const std::shared_ptr<RenderDevice> &renderDevice)
{
  // Shadow Pass Resource Set Layout (Set 0)
  _shadowPassLayout = renderDevice->createResourceSetLayout();
  _shadowPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // PerObjectBuffer
  _shadowPassLayout->addBinding(1, ResourceType::UNIFORM_BUFFER); // PerFrameBuffer
  _shadowPassLayout->build(renderDevice);

  // Point Light Depth Pass Resource Set Layout (Set 0)
  _pointLightDepthPassLayout = renderDevice->createResourceSetLayout();
  _pointLightDepthPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // PerObjectBuffer
  _pointLightDepthPassLayout->addBinding(1, ResourceType::UNIFORM_BUFFER); // PerFrameBuffer
  _pointLightDepthPassLayout->addBinding(2, ResourceType::UNIFORM_BUFFER); // PointLightBuffer
  _pointLightDepthPassLayout->build(renderDevice);

  // G-Buffer Pass Resource Set Layout (Set 0)
  _gbufferPassLayout = renderDevice->createResourceSetLayout();
  _gbufferPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // PerObjectBuffer
  _gbufferPassLayout->build(renderDevice);

  // Material Resource Set Layout (Set 1)
  _materialLayout = renderDevice->createResourceSetLayout();
  _materialLayout->addBinding(0, ResourceType::TEXTURE_2D); // DiffuseMap
  _materialLayout->addBinding(1, ResourceType::TEXTURE_2D); // NormalMap
  _materialLayout->addBinding(2, ResourceType::TEXTURE_2D); // MetallicMap
  _materialLayout->addBinding(3, ResourceType::TEXTURE_2D); // RoughnessMap
  _materialLayout->addBinding(4, ResourceType::TEXTURE_2D); // OcclusionMap
  _materialLayout->addBinding(5, ResourceType::TEXTURE_2D); // OpacityMap
  _materialLayout->addBinding(0, ResourceType::SAMPLER);    // DiffuseMap sampler
  _materialLayout->addBinding(1, ResourceType::SAMPLER);    // NormalMap sampler
  _materialLayout->addBinding(2, ResourceType::SAMPLER);    // MetallicMap sampler
  _materialLayout->addBinding(3, ResourceType::SAMPLER);    // RoughnessMap sampler
  _materialLayout->addBinding(4, ResourceType::SAMPLER);    // OcclusionMap sampler
  _materialLayout->addBinding(5, ResourceType::SAMPLER);    // OpacityMap sampler
  _materialLayout->build(renderDevice);

  // SSAO Pass Resource Set Layout (Set 0)
  _ssaoPassLayout = renderDevice->createResourceSetLayout();
  _ssaoPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // SsaoConstantsBuffer
  _ssaoPassLayout->addBinding(1, ResourceType::UNIFORM_BUFFER); // PerFrameBuffer
  _ssaoPassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // DepthMap (shader slot 0)
  _ssaoPassLayout->addBinding(1, ResourceType::TEXTURE_2D);     // NormalMap (shader slot 1)
  _ssaoPassLayout->addBinding(2, ResourceType::TEXTURE_2D);     // NoiseMap (shader slot 2)
  _ssaoPassLayout->addBinding(0, ResourceType::SAMPLER);        // DepthMap sampler
  _ssaoPassLayout->addBinding(1, ResourceType::SAMPLER);        // NormalMap sampler
  _ssaoPassLayout->addBinding(2, ResourceType::SAMPLER);        // NoiseMap sampler
  _ssaoPassLayout->build(renderDevice);

  // SSAO Blur Pass Resource Set Layout (Set 0)
  _ssaoBlurPassLayout = renderDevice->createResourceSetLayout();
  _ssaoBlurPassLayout->addBinding(0, ResourceType::TEXTURE_2D); // SsaoMap (shader slot 0)
  _ssaoBlurPassLayout->addBinding(0, ResourceType::SAMPLER);    // SsaoMap sampler
  _ssaoBlurPassLayout->build(renderDevice);

  // Lighting Pass Resource Set Layout (Set 0)
  _lightingPassLayout = renderDevice->createResourceSetLayout();
  _lightingPassLayout->addBinding(1, ResourceType::UNIFORM_BUFFER); // PerFrameBuffer (slot 1)
  _lightingPassLayout->addBinding(2, ResourceType::UNIFORM_BUFFER); // CascadeShadowMapBuffer (slot 2)
  _lightingPassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // AlbedoMap (slot 0)
  _lightingPassLayout->addBinding(1, ResourceType::TEXTURE_2D);     // DepthMap (slot 1)
  _lightingPassLayout->addBinding(2, ResourceType::TEXTURE_2D);     // NormalMap (slot 2)
  _lightingPassLayout->addBinding(3, ResourceType::TEXTURE_2D);     // MaterialMap (slot 3)
  _lightingPassLayout->addBinding(4, ResourceType::TEXTURE_2D);     // ShadowMap (slot 4)
  _lightingPassLayout->addBinding(5, ResourceType::TEXTURE_2D);     // OcclusionMap (slot 5)
  _lightingPassLayout->addBinding(6, ResourceType::TEXTURE_2D);     // ShadowMask (slot 6)
  _lightingPassLayout->addBinding(7, ResourceType::TEXTURE_2D);     // PointShadowMaps (slot 7)
  _lightingPassLayout->addBinding(8, ResourceType::TEXTURE_2D);     // RandomRotationsMap (slot 8)
  _lightingPassLayout->addBinding(0, ResourceType::SAMPLER);        // AlbedoMap sampler (slot 0)
  _lightingPassLayout->addBinding(1, ResourceType::SAMPLER);        // DepthMap sampler (slot 1)
  _lightingPassLayout->addBinding(2, ResourceType::SAMPLER);        // NormalMap sampler (slot 2)
  _lightingPassLayout->addBinding(3, ResourceType::SAMPLER);        // MaterialMap sampler (slot 3)
  _lightingPassLayout->addBinding(4, ResourceType::SAMPLER);        // ShadowMap sampler (slot 4)
  _lightingPassLayout->addBinding(5, ResourceType::SAMPLER);        // OcclusionMap sampler (slot 5)
  _lightingPassLayout->addBinding(6, ResourceType::SAMPLER);        // ShadowMask sampler (slot 6)
  _lightingPassLayout->addBinding(7, ResourceType::SAMPLER);        // PointShadowMaps sampler (slot 7)
  _lightingPassLayout->addBinding(8, ResourceType::SAMPLER);        // RandomRotationsMap sampler (slot 8)
  _lightingPassLayout->build(renderDevice);

  // Bloom Down Sample Pass Resource Set Layout (Set 0)
  _bloomDownSamplePassLayout = renderDevice->createResourceSetLayout();
  _bloomDownSamplePassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // BloomBuffer
  _bloomDownSamplePassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // Source texture
  _bloomDownSamplePassLayout->addBinding(0, ResourceType::SAMPLER);        // Sampler
  _bloomDownSamplePassLayout->build(renderDevice);

  // Bloom Up Sample Pass Resource Set Layout (Set 0)
  _bloomUpSamplePassLayout = renderDevice->createResourceSetLayout();
  _bloomUpSamplePassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // BloomBuffer
  _bloomUpSamplePassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // Source texture
  _bloomUpSamplePassLayout->addBinding(0, ResourceType::SAMPLER);        // Sampler
  _bloomUpSamplePassLayout->build(renderDevice);

  // Tone Mapping Pass Resource Set Layout (Set 0)
  _toneMappingPassLayout = renderDevice->createResourceSetLayout();
  _toneMappingPassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // LightingMap (texture slot 0)
  _toneMappingPassLayout->addBinding(1, ResourceType::TEXTURE_2D);     // BloomMap (texture slot 1)
  _toneMappingPassLayout->addBinding(0, ResourceType::SAMPLER);        // Sampler for LightingMap (sampler slot 0)
  _toneMappingPassLayout->addBinding(1, ResourceType::SAMPLER);        // Sampler for BloomMap (sampler slot 1)
  _toneMappingPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // PerFrameBuffer
  _toneMappingPassLayout->build(renderDevice);

  // Debug Pass Resource Set Layout (Set 0)
  _debugPassLayout = renderDevice->createResourceSetLayout();
  _debugPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // TexturedQuadBuffer (slot 0)
  _debugPassLayout->addBinding(0, ResourceType::TEXTURE_2D);     // Texture (slot 0)
  _debugPassLayout->addBinding(1, ResourceType::TEXTURE_2D);     // TextureArray (slot 1)
  _debugPassLayout->addBinding(2, ResourceType::TEXTURE_2D);     // TextureCubeArray (slot 2)
  _debugPassLayout->addBinding(0, ResourceType::SAMPLER);        // Sampler for Texture (slot 0)
  _debugPassLayout->addBinding(1, ResourceType::SAMPLER);        // Sampler for TextureArray (slot 1)
  _debugPassLayout->addBinding(2, ResourceType::SAMPLER);        // Sampler for TextureCubeArray (slot 2)
  _debugPassLayout->build(renderDevice);

  // AABB Pass Resource Set Layout (Set 0)
  _aabbPassLayout = renderDevice->createResourceSetLayout();
  _aabbPassLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // ObjectBuffer (slot 0)
  _aabbPassLayout->build(renderDevice);

  // Create Resource Sets (will be updated per frame)
  _shadowPassResourceSet = renderDevice->createResourceSet(_shadowPassLayout);
  _pointLightDepthPassResourceSet = renderDevice->createResourceSet(_pointLightDepthPassLayout);
  _gbufferPassResourceSet = renderDevice->createResourceSet(_gbufferPassLayout);
  _ssaoPassResourceSet = renderDevice->createResourceSet(_ssaoPassLayout);
  _ssaoBlurPassResourceSet = renderDevice->createResourceSet(_ssaoBlurPassLayout);
  _lightingPassResourceSet = renderDevice->createResourceSet(_lightingPassLayout);
  _bloomDownSamplePassResourceSet = renderDevice->createResourceSet(_bloomDownSamplePassLayout);
  _bloomUpSamplePassResourceSet = renderDevice->createResourceSet(_bloomUpSamplePassLayout);
  _toneMappingPassResourceSet = renderDevice->createResourceSet(_toneMappingPassLayout);
  _debugPassResourceSet = renderDevice->createResourceSet(_debugPassLayout);
  _aabbPassResourceSet = renderDevice->createResourceSet(_aabbPassLayout);

  // Build initial resource sets (will be rebuilt when resources change)
  updateResourceSets(renderDevice);
}

void Renderer::initCommandBuffers(const std::shared_ptr<RenderDevice> &renderDevice)
{
  // Create command buffers for each render pass
  _shadowCommandBuffer = renderDevice->createCommandBuffer();
  _pointLightDepthCommandBuffer = renderDevice->createCommandBuffer();
  _gbufferCommandBuffer = renderDevice->createCommandBuffer();
  _transparencyCommandBuffer = renderDevice->createCommandBuffer();
  _ssaoCommandBuffer = renderDevice->createCommandBuffer();
  _lightingCommandBuffer = renderDevice->createCommandBuffer();
  _bloomCommandBuffer = renderDevice->createCommandBuffer();
  _toneMappingCommandBuffer = renderDevice->createCommandBuffer();
  _debugCommandBuffer = renderDevice->createCommandBuffer();
}

void Renderer::updateResourceSets(const std::shared_ptr<RenderDevice> &renderDevice)
{
  // Reset and rebuild shadow pass resource set
  _shadowPassResourceSet->reset();
  _shadowPassResourceSet->addUniformBuffer(0, _perObjectBuffer);
  _shadowPassResourceSet->addUniformBuffer(1, _perFrameBuffer);
  _shadowPassResourceSet->build(renderDevice);

  // Reset and rebuild point light depth pass resource set
  _pointLightDepthPassResourceSet->reset();
  _pointLightDepthPassResourceSet->addUniformBuffer(0, _perObjectBuffer);
  _pointLightDepthPassResourceSet->addUniformBuffer(1, _perFrameBuffer);
  _pointLightDepthPassResourceSet->addUniformBuffer(2, _pointLightBuffer);
  _pointLightDepthPassResourceSet->build(renderDevice);

  // Reset and rebuild G-Buffer pass resource set
  _gbufferPassResourceSet->reset();
  _gbufferPassResourceSet->addUniformBuffer(0, _perObjectBuffer);
  _gbufferPassResourceSet->build(renderDevice);

  // Reset and rebuild SSAO pass resource set
  _ssaoPassResourceSet->reset();
  _ssaoPassResourceSet->addUniformBuffer(0, _ssaoConstantsBuffer);
  _ssaoPassResourceSet->addUniformBuffer(1, _perFrameBuffer);
  _ssaoPassResourceSet->addTexture(0, _gBufferDepthTex);   // DepthMap (shader slot 0)
  _ssaoPassResourceSet->addTexture(1, _gBufferNormalTex);  // NormalMap (shader slot 1)
  _ssaoPassResourceSet->addTexture(2, _ssaoNoiseTexture);  // NoiseMap (shader slot 2)
  _ssaoPassResourceSet->addSampler(0, _noMipSamplerState); // DepthMap sampler
  _ssaoPassResourceSet->addSampler(1, _noMipSamplerState); // NormalMap sampler
  _ssaoPassResourceSet->addSampler(2, _ssaoNoiseSampler);  // NoiseMap sampler
  _ssaoPassResourceSet->build(renderDevice);

  // Reset and rebuild SSAO blur pass resource set
  _ssaoBlurPassResourceSet->reset();
  _ssaoBlurPassResourceSet->addTexture(0, _ssaoTex);           // SsaoMap (shader slot 0)
  _ssaoBlurPassResourceSet->addSampler(0, _noMipSamplerState); // SsaoMap sampler
  _ssaoBlurPassResourceSet->build(renderDevice);

  // Reset and rebuild lighting pass resource set
  _lightingPassResourceSet->reset();
  _lightingPassResourceSet->addUniformBuffer(1, _perFrameBuffer);    // PerFrameBuffer (slot 1)
  _lightingPassResourceSet->addUniformBuffer(2, _perFrameBuffer);    // CascadeShadowMapBuffer (slot 2)
  _lightingPassResourceSet->addTexture(0, _gBufferAlbedoTex);        // AlbedoMap (slot 0)
  _lightingPassResourceSet->addTexture(1, _gBufferDepthTex);         // DepthMap (slot 1)
  _lightingPassResourceSet->addTexture(2, _gBufferNormalTex);        // NormalMap (slot 2)
  _lightingPassResourceSet->addTexture(3, _gBufferMaterialTex);      // MaterialMap (slot 3)
  _lightingPassResourceSet->addTexture(4, _shadowMapDepthTex);       // ShadowMap (slot 4)
  _lightingPassResourceSet->addTexture(5, _ssaoBlurTex);             // OcclusionMap (slot 5)
  _lightingPassResourceSet->addTexture(6, _shadowMapDepthTex);       // ShadowMask (slot 6)
  _lightingPassResourceSet->addTexture(7, _pointLightDepthTex);      // PointShadowMaps (slot 7)
  _lightingPassResourceSet->addTexture(8, _randomRotationsMap);      // RandomRotationsMap (slot 8)
  _lightingPassResourceSet->addSampler(0, _linearNoMipSamplerState); // AlbedoMap sampler (slot 0)
  _lightingPassResourceSet->addSampler(1, _noMipSamplerState);       // DepthMap sampler (slot 1)
  _lightingPassResourceSet->addSampler(2, _linearNoMipSamplerState); // NormalMap sampler (slot 2)
  _lightingPassResourceSet->addSampler(3, _linearNoMipSamplerState); // MaterialMap sampler (slot 3)
  _lightingPassResourceSet->addSampler(4, _shadowMapSamplerState);   // ShadowMap sampler (slot 4)
  _lightingPassResourceSet->addSampler(5, _linearNoMipSamplerState); // OcclusionMap sampler (slot 5)
  _lightingPassResourceSet->addSampler(6, _shadowMapSamplerState);   // ShadowMask sampler (slot 6)
  _lightingPassResourceSet->addSampler(7, _shadowMapSamplerState);   // PointShadowMaps sampler (slot 7)
  _lightingPassResourceSet->addSampler(8, _linearNoMipSamplerState); // RandomRotationsMap sampler (slot 8)
  _lightingPassResourceSet->build(renderDevice);

  // Reset and rebuild tone mapping pass resource set
  _toneMappingPassResourceSet->reset();
  _toneMappingPassResourceSet->addTexture(0, _lightingColorTex);
  _toneMappingPassResourceSet->addTexture(1, _bloomDownSampleTex[0]);
  _toneMappingPassResourceSet->addSampler(2, _noMipSamplerState);
  _toneMappingPassResourceSet->addSampler(3, _noMipSamplerState);
  _toneMappingPassResourceSet->addUniformBuffer(0, _perFrameBuffer);
  _toneMappingPassResourceSet->build(renderDevice);
}

void Renderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                         const std::vector<std::shared_ptr<DrawableComponent>> &allDrawables,
                         const std::vector<std::shared_ptr<LightComponent>> &lights,
                         const std::shared_ptr<CameraComponent> &camera)
{
  std::shared_ptr<LightComponent> directionalLight;
  for (const auto &light : lights)
  {
    if (light->getLightType() == LightComponentType::Directional)
    {
      directionalLight = light;
      break;
    }
  }
  if (!directionalLight)
  {
    throw std::runtime_error("No directional light found.");
  }

  // Perform frustum culling and object categorization
  std::vector<std::shared_ptr<DrawableComponent>> opaqueDrawables, transparentDrawables, aabbDrawables;
  performFrustumCulling(allDrawables, camera, opaqueDrawables, transparentDrawables, aabbDrawables);

  // Sort lights globally to ensure consistent ordering between shadow and lighting passes
  std::vector<std::shared_ptr<LightComponent>> sortedLights = lights; // Make a copy
  sortLightsForRendering(sortedLights, camera);

  writePerFrameConstantData(camera, directionalLight, sortedLights);

  if (directionalLight)
    directionalLightDepthPass(renderDevice, allDrawables, directionalLight, camera);
  pointLightDepthPass(renderDevice, allDrawables, sortedLights, camera);

  // G-Buffer pass for deferred rendering
  gbufferPass(renderDevice, opaqueDrawables, camera);

  transparencyPass(renderDevice, transparentDrawables, camera);
  // if (directionalLight) shadowPass(renderDevice);  // Commented out - using direct shadows now
  ssaoPass(renderDevice, camera);
  lightingPass(renderDevice, sortedLights, camera);
  bloomPass(renderDevice);
  toneMappingPass(renderDevice);
  debugPass(renderDevice, aabbDrawables, camera);
}

void Renderer::initSamplers(const std::shared_ptr<RenderDevice> &renderDevice)
{
  SamplerStateDesc basicSamplerStateDesc;
  basicSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Repeat, TextureAddressMode::Repeat, TextureAddressMode::Repeat};
  basicSamplerStateDesc.MinFiltering = TextureFilteringMode::LinearMipLinear;
  basicSamplerStateDesc.MagFiltering = TextureFilteringMode::Linear;
  basicSamplerStateDesc.MaxAnisotropy = 16.0f; // High-quality anisotropic filtering
  _basicSamplerState = renderDevice->createSamplerState(basicSamplerStateDesc);

  SamplerStateDesc noMipSamplerState;
  noMipSamplerState.AddressingMode = AddressingMode{TextureAddressMode::Repeat, TextureAddressMode::Repeat, TextureAddressMode::Repeat};
  noMipSamplerState.MinFiltering = TextureFilteringMode::Nearest;
  noMipSamplerState.MagFiltering = TextureFilteringMode::Nearest;
  _noMipSamplerState = renderDevice->createSamplerState(noMipSamplerState);

  SamplerStateDesc shadowMapSamplerStateDesc;
  shadowMapSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::ClampToBorder, TextureAddressMode::ClampToBorder, TextureAddressMode::ClampToBorder};
  shadowMapSamplerStateDesc.MinFiltering = TextureFilteringMode::Nearest;
  shadowMapSamplerStateDesc.MagFiltering = TextureFilteringMode::Nearest;
  shadowMapSamplerStateDesc.BorderColour = Colour::White;
  _shadowMapSamplerState = renderDevice->createSamplerState(shadowMapSamplerStateDesc);

  SamplerStateDesc ssaoNoiseSamplerDesc;
  ssaoNoiseSamplerDesc.AddressingMode = AddressingMode{TextureAddressMode::Repeat, TextureAddressMode::Repeat, TextureAddressMode::Repeat};
  ssaoNoiseSamplerDesc.MinFiltering = TextureFilteringMode::Nearest;
  ssaoNoiseSamplerDesc.MagFiltering = TextureFilteringMode::Nearest;
  _ssaoNoiseSampler = renderDevice->createSamplerState(ssaoNoiseSamplerDesc);

  SamplerStateDesc noMipWithBorderSamplerState;
  noMipWithBorderSamplerState.AddressingMode = AddressingMode{TextureAddressMode::ClampToBorder, TextureAddressMode::ClampToBorder, TextureAddressMode::ClampToBorder};
  noMipWithBorderSamplerState.MinFiltering = TextureFilteringMode::Nearest;
  noMipWithBorderSamplerState.MagFiltering = TextureFilteringMode::Nearest;
  _noMipWithBorderSamplerState = renderDevice->createSamplerState(noMipWithBorderSamplerState);

  SamplerStateDesc bloomSamplerStateDesc;
  bloomSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::ClampToEdge, TextureAddressMode::ClampToEdge, TextureAddressMode::ClampToEdge};
  bloomSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
  bloomSamplerStateDesc.MagFiltering = TextureFilteringMode::Linear;
  _bloomSamplerState = renderDevice->createSamplerState(bloomSamplerStateDesc);

  SamplerStateDesc linearNoMipSamplerStateDesc;
  linearNoMipSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Repeat, TextureAddressMode::Repeat, TextureAddressMode::Repeat};
  linearNoMipSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
  linearNoMipSamplerStateDesc.MagFiltering = TextureFilteringMode::Linear;
  _linearNoMipSamplerState = renderDevice->createSamplerState(linearNoMipSamplerStateDesc);
}

void Renderer::initTextures(const std::shared_ptr<RenderDevice> &renderDevice)
{
  // Create default white texture (1x1 white pixel) with mipmaps
  TextureDesc whiteTextureDesc;
  whiteTextureDesc.Width = 1;
  whiteTextureDesc.Height = 1;
  whiteTextureDesc.Format = TextureFormat::RGBA8;
  whiteTextureDesc.Usage = TextureUsage::Default;
  whiteTextureDesc.Type = TextureType::Texture2D;
  whiteTextureDesc.MipLevels = 1; // 1x1 texture only needs 1 mip level
  _defaultWhiteTexture = renderDevice->createTexture(whiteTextureDesc);

  uint32 whitePixel = 0xFFFFFFFF;
  _defaultWhiteTexture->writeData(0, 0, 0, 1, 0, 1, 0, 1, &whitePixel);

  // Create default normal texture (1x1 normal pointing up: RGB(128, 128, 255)) with mipmaps
  TextureDesc normalTextureDesc;
  normalTextureDesc.Width = 1;
  normalTextureDesc.Height = 1;
  normalTextureDesc.Format = TextureFormat::RGBA8;
  normalTextureDesc.Usage = TextureUsage::Default;
  normalTextureDesc.Type = TextureType::Texture2D;
  normalTextureDesc.MipLevels = 1; // 1x1 texture only needs 1 mip level
  _defaultNormalTexture = renderDevice->createTexture(normalTextureDesc);

  uint32 normalPixel = 0xFF8080FF; // ABGR format: A=255, B=128, G=128, R=255
  _defaultNormalTexture->writeData(0, 0, 0, 1, 0, 1, 0, 1, &normalPixel);

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

  std::uniform_real_distribution<float32> randomFloats(0.0f, 1.0f);
  // use global SSAO RNG to keep noise texture stable when reloading
  auto &generator = g_ssaoGenerator;

  std::vector<Vector3> ssaoNoise;
  for (uint32 i = 0; i < SSAO_NOISE_TEXTURE_SIZE * SSAO_NOISE_TEXTURE_SIZE; i++)
  {
    Vector3 noise(randomFloats(generator, randomFloats.param()) * 2.0f - 1.0f,
                  randomFloats(generator, randomFloats.param()) * 2.0f - 1.0f,
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
  _shadowMapDepthTex = renderDevice->createTexture(shadowMapDesc);
  {
    FramebufferDesc fb{};
    fb.width = _shadowMapResolution;
    fb.height = _shadowMapResolution;
    fb.samples = 1;
    fb.depthStencilAttachment = FramebufferAttachment{_shadowMapDepthTex};
    fb.hasDepthStencilAttachment = true;
    _shadowMapFb = std::make_shared<Framebuffer>(fb);
  }

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

  GraphicsPipelineStateDesc shadowDesc{};
  shadowDesc.VS = renderDevice->createShader(vsDesc);
  shadowDesc.GS = renderDevice->createShader(gsDesc);
  shadowDesc.FS = renderDevice->createShader(psDesc);
  shadowDesc.Blend = renderDevice->createBlendState(BlendStateDesc{});
  shadowDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  shadowDesc.DepthStencil = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  shadowDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  shadowDesc.ShaderParamReflection = shaderParams;
  shadowDesc.Topology = PrimitiveTopology::TriangleList;
  shadowDesc.HasDepthStencil = true;
  shadowDesc.Samples = 1;
  _shadowMapPso = std::make_shared<GraphicsPipelineState>(shadowDesc);
}

void Renderer::initPointLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  TextureDesc shadowMapDesc;
  shadowMapDesc.Width = _pointLightShadowMapResolution;
  shadowMapDesc.Height = _pointLightShadowMapResolution;
  shadowMapDesc.Usage = TextureUsage::Depth;
  shadowMapDesc.Type = TextureType::TextureCubeArray;
  shadowMapDesc.Format = TextureFormat::D32F;
  shadowMapDesc.Count = MAX_POINT_LIGHT_SHADOW_CASTERS;
  _pointLightDepthTex = renderDevice->createTexture(shadowMapDesc);
  {
    FramebufferDesc fb{};
    fb.width = _pointLightShadowMapResolution;
    fb.height = _pointLightShadowMapResolution;
    fb.samples = 1;
    fb.depthStencilAttachment = FramebufferAttachment{_pointLightDepthTex};
    fb.hasDepthStencilAttachment = true;
    _pointLightDepthFb = std::make_shared<Framebuffer>(fb);
  }

  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/PointShadowMap.vert");

  ShaderDesc gsDesc;
  gsDesc.ShaderType = ShaderType::Geometry;
  gsDesc.Source = String::foadFromFile("./Shaders/PointShadowMap.geom");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/PointShadowMap.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));
  shaderParams->addParam(ShaderParam("PointLightBuffer", ShaderParamType::ConstBuffer, 2));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::Clockwise;

  GraphicsPipelineStateDesc pointShadowDesc{};
  pointShadowDesc.VS = renderDevice->createShader(vsDesc);
  pointShadowDesc.GS = renderDevice->createShader(gsDesc);
  pointShadowDesc.FS = renderDevice->createShader(psDesc);
  pointShadowDesc.Blend = renderDevice->createBlendState(BlendStateDesc{});
  pointShadowDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  pointShadowDesc.DepthStencil = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  pointShadowDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  pointShadowDesc.ShaderParamReflection = shaderParams;
  pointShadowDesc.Topology = PrimitiveTopology::TriangleList;
  pointShadowDesc.HasDepthStencil = true;
  _pointLightDepthPso = std::make_shared<GraphicsPipelineState>(pointShadowDesc);
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
  shaderParams->addParam(ShaderParam("MetallicMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("RoughnessMap", ShaderParamType::Texture, 3));
  shaderParams->addParam(ShaderParam("OcclusionMap", ShaderParamType::Texture, 4));
  shaderParams->addParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 5));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

  BlendStateDesc blendStateDesc{};

  GraphicsPipelineStateDesc gbufferDesc{};
  gbufferDesc.VS = renderDevice->createShader(vsDesc);
  gbufferDesc.FS = renderDevice->createShader(psDesc);
  gbufferDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  gbufferDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  gbufferDesc.DepthStencil = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  gbufferDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  gbufferDesc.ShaderParamReflection = shaderParams;
  gbufferDesc.Topology = PrimitiveTopology::TriangleList;
  gbufferDesc.HasDepthStencil = true;
  _gBufferPso = std::make_shared<GraphicsPipelineState>(gbufferDesc);

  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::RGBA8;

  // Use higher precision format for albedo target to reduce banding
  TextureDesc albedoTexDesc = colourTexDesc;
  albedoTexDesc.Format = TextureFormat::RGBA16F;

  TextureDesc depthStencilDesc;
  depthStencilDesc.Width = _windowDims.X;
  depthStencilDesc.Height = _windowDims.Y;
  depthStencilDesc.Usage = TextureUsage::Depth;
  depthStencilDesc.Type = TextureType::Texture2D;
  depthStencilDesc.Format = TextureFormat::D24;

  _gBufferAlbedoTex = renderDevice->createTexture(albedoTexDesc);
  _gBufferNormalTex = renderDevice->createTexture(colourTexDesc);
  _gBufferMaterialTex = renderDevice->createTexture(colourTexDesc);
  _gBufferDepthTex = renderDevice->createTexture(depthStencilDesc);
  {
    FramebufferDesc fb{};
    fb.width = _windowDims.X;
    fb.height = _windowDims.Y;
    fb.samples = 1;
    fb.colorAttachments = {
        FramebufferAttachment{_gBufferAlbedoTex},
        FramebufferAttachment{_gBufferNormalTex},
        FramebufferAttachment{_gBufferMaterialTex}};
    fb.depthStencilAttachment = FramebufferAttachment{_gBufferDepthTex};
    fb.hasDepthStencilAttachment = true;
    _gBufferFb = std::make_shared<Framebuffer>(fb);
  }
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
  shaderParams->addParam(ShaderParam("MetallicMap", ShaderParamType::Texture, 2));
  shaderParams->addParam(ShaderParam("RoughnessMap", ShaderParamType::Texture, 3));
  shaderParams->addParam(ShaderParam("OcclusionMap", ShaderParamType::Texture, 4));
  shaderParams->addParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 5));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

  BlendStateDesc blendStateDesc{};
  blendStateDesc.RTBlendState[0].BlendEnabled = true;
  blendStateDesc.RTBlendState[0].BlendAlpha = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

  GraphicsPipelineStateDesc transparencyDesc{};
  transparencyDesc.VS = renderDevice->createShader(vsDesc);
  transparencyDesc.FS = renderDevice->createShader(psDesc);
  transparencyDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  transparencyDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  transparencyDesc.DepthStencil = renderDevice->createDepthStencilState(DepthStencilStateDesc());
  transparencyDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  transparencyDesc.ShaderParamReflection = shaderParams;
  transparencyDesc.Topology = PrimitiveTopology::TriangleList;
  transparencyDesc.HasDepthStencil = true;
  _transparencyPso = std::make_shared<GraphicsPipelineState>(transparencyDesc);
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
    shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));
    shaderParams->addParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
    shaderParams->addParam(ShaderParam("NoiseMap", ShaderParamType::Texture, 2));

    RasterizerStateDesc rasterizerStateDesc{};
    BlendStateDesc blendStateDesc{};

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    GraphicsPipelineStateDesc ssaoDesc{};
    ssaoDesc.VS = renderDevice->createShader(vsDesc);
    ssaoDesc.FS = renderDevice->createShader(psDesc);
    ssaoDesc.Blend = renderDevice->createBlendState(blendStateDesc);
    ssaoDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
    ssaoDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
    ssaoDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
    ssaoDesc.ShaderParamReflection = shaderParams;
    ssaoDesc.Topology = PrimitiveTopology::TriangleList;
    ssaoDesc.HasDepthStencil = true;
    _ssaoPso = std::make_shared<GraphicsPipelineState>(ssaoDesc);
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

    GraphicsPipelineStateDesc ssaoBlurDesc{};
    ssaoBlurDesc.VS = renderDevice->createShader(vsDesc);
    ssaoBlurDesc.FS = renderDevice->createShader(psDesc);
    ssaoBlurDesc.Blend = renderDevice->createBlendState(blendStateDesc);
    ssaoBlurDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
    ssaoBlurDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
    ssaoBlurDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
    ssaoBlurDesc.ShaderParamReflection = shaderParams;
    ssaoBlurDesc.Topology = PrimitiveTopology::TriangleList;
    ssaoBlurDesc.HasDepthStencil = true;
    _ssaoBlurPso = std::make_shared<GraphicsPipelineState>(ssaoBlurDesc);
  }
  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::R8;

  _ssaoTex = renderDevice->createTexture(colourTexDesc);
  _ssaoBlurTex = renderDevice->createTexture(colourTexDesc);
  {
    FramebufferDesc fb{};
    fb.width = _windowDims.X;
    fb.height = _windowDims.Y;
    fb.samples = 1;
    fb.colorAttachments = {FramebufferAttachment{_ssaoTex}};
    _ssaoFb = std::make_shared<Framebuffer>(fb);
  }
  {
    FramebufferDesc fb{};
    fb.width = _windowDims.X;
    fb.height = _windowDims.Y;
    fb.samples = 1;
    fb.colorAttachments = {FramebufferAttachment{_ssaoBlurTex}};
    _ssaoBlurFb = std::make_shared<Framebuffer>(fb);
  }
}

void Renderer::initLightingPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/PbrLighting.frag");

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
  shaderParams->addParam(ShaderParam("MaterialMap", ShaderParamType::Texture, 3));
  shaderParams->addParam(ShaderParam("ShadowMap", ShaderParamType::Texture, 4));
  shaderParams->addParam(ShaderParam("OcclusionMap", ShaderParamType::Texture, 5));

  // Expose directional shadow mask from shadow pass
  shaderParams->addParam(ShaderParam("ShadowMask", ShaderParamType::Texture, 6));
  // Expose point-light shadow cubemap for point shadows
  shaderParams->addParam(ShaderParam("PointShadowMaps", ShaderParamType::Texture, 7));
  // Expose random rotations map for Poisson sampling
  shaderParams->addParam(ShaderParam("RandomRotationsMap", ShaderParamType::Texture, 8));

  RasterizerStateDesc rasterizerStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  BlendStateDesc blendStateDesc{};

  GraphicsPipelineStateDesc lightingDesc{};
  lightingDesc.VS = renderDevice->createShader(vsDesc);
  lightingDesc.FS = renderDevice->createShader(psDesc);
  lightingDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  lightingDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  lightingDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
  lightingDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  lightingDesc.ShaderParamReflection = shaderParams;
  lightingDesc.Topology = PrimitiveTopology::TriangleList;
  lightingDesc.HasDepthStencil = true;
  _lightingPso = std::make_shared<GraphicsPipelineState>(lightingDesc);
  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::RGB16F;

  TextureDesc bloomTexDesc;
  bloomTexDesc.Width = _windowDims.X;
  bloomTexDesc.Height = _windowDims.Y;
  bloomTexDesc.Usage = TextureUsage::RenderTarget;
  bloomTexDesc.Type = TextureType::Texture2D;
  bloomTexDesc.Format = TextureFormat::RGB16F;

  _lightingColorTex = renderDevice->createTexture(colourTexDesc);
  _lightingBloomTex = renderDevice->createTexture(bloomTexDesc);
  {
    FramebufferDesc fb{};
    fb.width = _windowDims.X;
    fb.height = _windowDims.Y;
    fb.samples = 1;
    fb.colorAttachments = {FramebufferAttachment{_lightingColorTex}, FramebufferAttachment{_lightingBloomTex}};
    _lightingPassFb = std::make_shared<Framebuffer>(fb);
  }
}

void Renderer::initBloomDownSamplePass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/BlurDownSample.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
  };

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("BloomBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("Texture", ShaderParamType::Texture, 0));

  RasterizerStateDesc rasterizerStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  BlendStateDesc blendStateDesc{};

  GraphicsPipelineStateDesc bloomDownDesc{};
  bloomDownDesc.VS = renderDevice->createShader(vsDesc);
  bloomDownDesc.FS = renderDevice->createShader(psDesc);
  bloomDownDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  bloomDownDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  bloomDownDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
  bloomDownDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  bloomDownDesc.ShaderParamReflection = shaderParams;
  bloomDownDesc.Topology = PrimitiveTopology::TriangleList;
  bloomDownDesc.HasDepthStencil = true;
  _bloomDownSamplePso = std::make_shared<GraphicsPipelineState>(bloomDownDesc);

  _bloomDownSampleFbs.clear();
  _bloomDownSampleTex.clear();
  Vector2I textureSize(_windowDims.X, _windowDims.Y);
  for (uint32 i = 0; i < 6; i++)
  {
    textureSize /= 2;

    TextureDesc mipTextureDesc;
    mipTextureDesc.Width = textureSize.X;
    mipTextureDesc.Height = textureSize.Y;
    mipTextureDesc.Usage = TextureUsage::RenderTarget;
    mipTextureDesc.Type = TextureType::Texture2D;
    mipTextureDesc.Format = TextureFormat::RGB16F;

    auto tex = renderDevice->createTexture(mipTextureDesc);
    _bloomDownSampleTex.push_back(tex);
    FramebufferDesc fb{};
    fb.width = textureSize.X;
    fb.height = textureSize.Y;
    fb.samples = 1;
    fb.colorAttachments = {FramebufferAttachment{tex}};
    _bloomDownSampleFbs.push_back(std::make_shared<Framebuffer>(fb));
  }
}

void Renderer::initBloomUpSamplePass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/BlurUpSample.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
  };

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("BloomBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->addParam(ShaderParam("Texture", ShaderParamType::Texture, 0));

  RasterizerStateDesc rasterizerStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  BlendStateDesc blendStateDesc;
  blendStateDesc.RTBlendState[0].BlendEnabled = true;
  blendStateDesc.RTBlendState[0].Blend = BlendDesc(BlendFactor::One, BlendFactor::One, BlendOperation::Add);

  GraphicsPipelineStateDesc bloomUpDesc{};
  bloomUpDesc.VS = renderDevice->createShader(vsDesc);
  bloomUpDesc.FS = renderDevice->createShader(psDesc);
  bloomUpDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  bloomUpDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  bloomUpDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
  bloomUpDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  bloomUpDesc.ShaderParamReflection = shaderParams;
  bloomUpDesc.Topology = PrimitiveTopology::TriangleList;
  bloomUpDesc.HasDepthStencil = true;
  _bloomUpPso = std::make_shared<GraphicsPipelineState>(bloomUpDesc);
}

void Renderer::initToneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  ShaderDesc vsDesc;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

  ShaderDesc psDesc;
  psDesc.ShaderType = ShaderType::Fragment;
  psDesc.Source = String::foadFromFile("./Shaders/ToneMapping.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
  };

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->addParam(ShaderParam("PerFrameBuffer", ShaderParamType::ConstBuffer, 1));
  shaderParams->addParam(ShaderParam("LightingMap", ShaderParamType::Texture, 0));
  shaderParams->addParam(ShaderParam("BloomMap", ShaderParamType::Texture, 1));

  RasterizerStateDesc rasterizerStateDesc{};
  BlendStateDesc blendStateDesc{};

  DepthStencilStateDesc depthStencilStateDesc{};
  depthStencilStateDesc.DepthReadEnabled = false;
  depthStencilStateDesc.DepthWriteEnabled = false;

  GraphicsPipelineStateDesc toneDesc{};
  toneDesc.VS = renderDevice->createShader(vsDesc);
  toneDesc.FS = renderDevice->createShader(psDesc);
  toneDesc.Blend = renderDevice->createBlendState(blendStateDesc);
  toneDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
  toneDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
  toneDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
  toneDesc.ShaderParamReflection = shaderParams;
  toneDesc.Topology = PrimitiveTopology::TriangleList;
  toneDesc.HasDepthStencil = true;
  _toneMappingPso = std::make_shared<GraphicsPipelineState>(toneDesc);

  TextureDesc colourTexDesc;
  colourTexDesc.Width = _windowDims.X;
  colourTexDesc.Height = _windowDims.Y;
  colourTexDesc.Usage = TextureUsage::RenderTarget;
  colourTexDesc.Type = TextureType::Texture2D;
  colourTexDesc.Format = TextureFormat::RGB8;

  _toneMappingColorTex = renderDevice->createTexture(colourTexDesc);
  {
    FramebufferDesc fb{};
    fb.width = _windowDims.X;
    fb.height = _windowDims.Y;
    fb.samples = 1;
    fb.colorAttachments = {FramebufferAttachment{_toneMappingColorTex}};
    _toneMappingFb = std::make_shared<Framebuffer>(fb);
  }
}

void Renderer::initDebugPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  {
    ShaderDesc vsDesc;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::foadFromFile("./Shaders/FSPassThrough.vert");

    ShaderDesc psDesc;
    psDesc.ShaderType = ShaderType::Fragment;
    psDesc.Source = String::foadFromFile("./Shaders/Editor/DrawTexturedQuad.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
        VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
    };

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->addParam(ShaderParam("Texture", ShaderParamType::Texture, 0));
    shaderParams->addParam(ShaderParam("TextureArray", ShaderParamType::Texture, 1));
    shaderParams->addParam(ShaderParam("TextureCubeArray", ShaderParamType::Texture, 2));
    shaderParams->addParam(ShaderParam("TexturedQuadBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    // legacy pipeline removed - create editor textured quad pipeline state
    GraphicsPipelineStateDesc editorQuadDesc{};
    editorQuadDesc.VS = renderDevice->createShader(vsDesc);
    editorQuadDesc.FS = renderDevice->createShader(psDesc);
    editorQuadDesc.Blend = renderDevice->createBlendState(BlendStateDesc());
    editorQuadDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
    editorQuadDesc.DepthStencil = renderDevice->createDepthStencilState(DepthStencilStateDesc());
    editorQuadDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
    editorQuadDesc.ShaderParamReflection = shaderParams;
    editorQuadDesc.Topology = PrimitiveTopology::TriangleList;
    editorQuadDesc.HasDepthStencil = true;
    _editorDrawTexturedQuadPso = std::make_shared<GraphicsPipelineState>(editorQuadDesc);
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
    depthStencilStateDesc.DepthReadEnabled = true;
    depthStencilStateDesc.DepthWriteEnabled = false;

    BlendStateDesc blendStateDesc{};

    // legacy pipeline removed - create draw AABB pipeline state
    GraphicsPipelineStateDesc aabbDesc{};
    aabbDesc.VS = renderDevice->createShader(vsDesc);
    aabbDesc.FS = renderDevice->createShader(psDesc);
    aabbDesc.Blend = renderDevice->createBlendState(blendStateDesc);
    aabbDesc.Rasterizer = renderDevice->createRasterizerState(rasterizerStateDesc);
    aabbDesc.DepthStencil = renderDevice->createDepthStencilState(depthStencilStateDesc);
    aabbDesc.VertexLayoutDef = renderDevice->createVertexLayout(vertexLayoutDesc);
    aabbDesc.ShaderParamReflection = shaderParams;
    aabbDesc.Topology = PrimitiveTopology::LineList;
    aabbDesc.HasDepthStencil = true;
    _drawAabbPso = std::make_shared<GraphicsPipelineState>(aabbDesc);
  }
}

void Renderer::directionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                         const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                                         const std::shared_ptr<LightComponent> &directionalLight,
                                         const std::shared_ptr<CameraComponent> &camera)
{
  if (_shadowResolutionChanged)
  {
    createDirectionalLightShadowDepthMap(renderDevice);
  }

  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Setup shadow frustum culling
  std::vector<Matrix4> lightTransforms = calculateCascadeLightTransforms(camera, directionalLight);
  _shadowFrustum->buildFromLightTransforms(lightTransforms, _cascadeCount);
  _shadowFrustum->buildExtendedCameraFrustum(*camera, _maxCascadeDistance);

  // Perform shadow culling
  std::vector<std::shared_ptr<DrawableComponent>> broadPhaseCulled = _shadowFrustum->broadPhaseCull(drawables);
  std::vector<std::shared_ptr<DrawableComponent>> shadowCasters = _shadowFrustum->shadowRelevanceFilter(broadPhaseCulled);

  // Setup shadow queue and sort for optimal rendering
  _shadowQueue->clear();
  for (const auto &drawable : shadowCasters)
  {
    _shadowQueue->add(drawable);
  }
  _shadowQueue->sort(*camera);

  // Begin command buffer recording
  _shadowCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set pipeline state first (to match original order)
  _shadowCommandBuffer->bindGraphicsPipeline(_shadowMapPso);

  // Set viewport before beginning render pass
  ViewportDesc viewportDesc;
  viewportDesc.Height = _shadowMapResolution;
  viewportDesc.Width = _shadowMapResolution;
  _shadowCommandBuffer->setViewport(viewportDesc);

  // Begin render pass
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _shadowMapFb;
    // depth-only clear
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    _shadowCommandBuffer->beginRenderPass(rp);
  }
  _shadowCommandBuffer->bindResourceSet(_shadowPassResourceSet, 0);

  // Render culled and sorted shadow casters
  for (const auto &drawable : _shadowQueue->getDrawables())
  {
    std::shared_ptr<Material> material(drawable->getMaterial());
    drawDrawable(_shadowCommandBuffer, renderDevice, drawable, material, camera);
  }

  // End render pass and command buffer
  _shadowCommandBuffer->endRenderPass();
  _shadowCommandBuffer->end();
  SubmitRecorded(renderDevice, _shadowCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[0].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::gbufferPass(const std::shared_ptr<RenderDevice> &renderDevice,
                           const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                           const std::shared_ptr<CameraComponent> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Begin command buffer recording
  _gbufferCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Begin render pass (new API path)
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _gBufferFb;
    rp.clearColors = {ClearColorValue{0, 0, 0, 1}}; // clear first color
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    _gbufferCommandBuffer->beginRenderPass(rp);
  }

  // Set viewport
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  _gbufferCommandBuffer->setViewport(viewportDesc);

  // Set pipeline state and bind resource sets
  _gbufferCommandBuffer->bindGraphicsPipeline(_gBufferPso);
  _gbufferCommandBuffer->bindResourceSet(_gbufferPassResourceSet, 0);

  // Draw all drawables
  for (const auto &drawable : drawables)
  {
    auto material = drawable->getMaterial();
    if (!material)
      continue;

    // Create and bind material resource set
    auto materialResourceSet = createMaterialResourceSet(renderDevice, material);
    _gbufferCommandBuffer->bindResourceSet(materialResourceSet, 1);

    drawDrawable(_gbufferCommandBuffer, renderDevice, drawable, material, camera);
  }

  // End render pass and command buffer
  _gbufferCommandBuffer->endRenderPass();
  _gbufferCommandBuffer->end();
  SubmitRecorded(renderDevice, _gbufferCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[1].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::transparencyPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                const std::vector<std::shared_ptr<DrawableComponent>> &transparentDrawables,
                                const std::shared_ptr<CameraComponent> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Begin command buffer recording
  _transparencyCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set pipeline state
  _transparencyCommandBuffer->bindGraphicsPipeline(_transparencyPso);

  // Begin render pass (continuing from gbuffer pass)
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _gBufferFb;
    // no clears for transparency overlay
    _transparencyCommandBuffer->beginRenderPass(rp);
  }
  _transparencyCommandBuffer->bindResourceSet(_gbufferPassResourceSet, 0);

  // Draw all transparent drawables
  for (const auto &drawable : transparentDrawables)
  {
    auto material = drawable->getMaterial();
    if (!material)
      continue;

    auto materialResourceSet = createMaterialResourceSet(renderDevice, material);
    _transparencyCommandBuffer->bindResourceSet(materialResourceSet, 1);

    drawDrawable(_transparencyCommandBuffer, renderDevice, drawable, material, camera);
  }

  // End render pass and command buffer
  _transparencyCommandBuffer->endRenderPass();
  _transparencyCommandBuffer->end();
  SubmitRecorded(renderDevice, _transparencyCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[2].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::shared_ptr<CameraComponent> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  if (_ssaoSettingsModified)
  {
    writeSsaoConstantData(renderDevice, camera);
    _ssaoSettingsModified = false;
  }

  // Begin command buffer recording
  _ssaoCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // SSAO generation pass
  _ssaoCommandBuffer->bindGraphicsPipeline(_ssaoPso);
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _ssaoFb;
    rp.clearColors = {ClearColorValue{0, 0, 0, 0}}; // clear only color
    _ssaoCommandBuffer->beginRenderPass(rp);
  }
  _ssaoCommandBuffer->bindResourceSet(_ssaoPassResourceSet, 0);
  _ssaoCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  _ssaoCommandBuffer->draw(6, 0);
  _ssaoCommandBuffer->endRenderPass();

  // SSAO blur pass
  _ssaoCommandBuffer->bindGraphicsPipeline(_ssaoBlurPso);
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _ssaoBlurFb;
    rp.clearColors = {ClearColorValue{0, 0, 0, 0}}; // clear only color
    _ssaoCommandBuffer->beginRenderPass(rp);
  }
  _ssaoCommandBuffer->bindResourceSet(_ssaoBlurPassResourceSet, 0);
  _ssaoCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  _ssaoCommandBuffer->draw(6, 0);
  _ssaoCommandBuffer->endRenderPass();

  // End command buffer and execute
  _ssaoCommandBuffer->end();
  SubmitRecorded(renderDevice, _ssaoCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[4].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::lightingPass(const std::shared_ptr<RenderDevice> &renderDevice,
                            const std::vector<std::shared_ptr<LightComponent>> &lights,
                            const std::shared_ptr<CameraComponent> &camera)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Begin command buffer recording
  _lightingCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set pipeline state
  _lightingCommandBuffer->bindGraphicsPipeline(_lightingPso);

  // Begin render pass
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _lightingPassFb;
    rp.clearColors = {ClearColorValue{0, 0, 0, 0}, ClearColorValue{0, 0, 0, 0}}; // lighting + bloom targets
    _lightingCommandBuffer->beginRenderPass(rp);
  }
  _lightingCommandBuffer->bindResourceSet(_lightingPassResourceSet, 0);
  _lightingCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  _lightingCommandBuffer->draw(6, 0);

  // End render pass and command buffer
  _lightingCommandBuffer->endRenderPass();
  _lightingCommandBuffer->end();
  SubmitRecorded(renderDevice, _lightingCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[5].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::bloomPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Begin command buffer recording
  _bloomCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Downsample pass
  _bloomCommandBuffer->bindGraphicsPipeline(_bloomDownSamplePso);

  // Progressively downsample through the bloom mip chain.
  for (size_t i = 0; i < _bloomDownSampleFbs.size(); ++i)
  {
    const auto &bloomDownSampleFb = _bloomDownSampleFbs[i];

    ViewportDesc viewportDesc;
    viewportDesc.Width = bloomDownSampleFb->getDesc().width;
    viewportDesc.Height = bloomDownSampleFb->getDesc().height;
    _bloomCommandBuffer->setViewport(viewportDesc);

    BloomBuffer bufferData;
    bufferData.FilterRadius = _bloomFilter;
    bufferData.SourceResolution = Vector2(viewportDesc.Width, viewportDesc.Height);
    _bloomBuffer->writeData(0, sizeof(BloomBuffer), &bufferData, AccessType::WriteOnlyDiscard);

    // Update and bind resource set for this downsample iteration
    _bloomDownSamplePassResourceSet->reset();
    _bloomDownSamplePassResourceSet->addUniformBuffer(0, _bloomBuffer);
    if (i == 0)
    {
      _bloomDownSamplePassResourceSet->addTexture(0, _lightingBloomTex);
    }
    else
    {
      _bloomDownSamplePassResourceSet->addTexture(0, _bloomDownSampleTex[i - 1]);
    }
    _bloomDownSamplePassResourceSet->addSampler(0, _bloomSamplerState);
    _bloomDownSamplePassResourceSet->build(renderDevice);

    {
      RenderPassBeginInfo rp{};
      rp.framebuffer = bloomDownSampleFb;
      rp.clearColors = {ClearColorValue{0, 0, 0, 0}};
      _bloomCommandBuffer->beginRenderPass(rp);
    }
    _bloomCommandBuffer->bindResourceSet(_bloomDownSamplePassResourceSet, 0);
    _bloomCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
    _bloomCommandBuffer->draw(6, 0);
    _bloomCommandBuffer->endRenderPass();
  }

  // Upsample pass
  _bloomCommandBuffer->bindGraphicsPipeline(_bloomUpPso);

  for (uint32 i = static_cast<uint32>(_bloomDownSampleFbs.size()) - 1; i > 0; i--)
  {
    const auto &currentFb = _bloomDownSampleFbs[i];
    const auto &nextFb = _bloomDownSampleFbs[i - 1];

    // Update and bind resource set for this upsample iteration
    _bloomUpSamplePassResourceSet->reset();
    _bloomUpSamplePassResourceSet->addUniformBuffer(0, _bloomBuffer);
    _bloomUpSamplePassResourceSet->addTexture(0, _bloomDownSampleTex[i]);
    _bloomUpSamplePassResourceSet->addSampler(0, _bloomSamplerState);
    _bloomUpSamplePassResourceSet->build(renderDevice);

    ViewportDesc viewportDesc{};
    viewportDesc.Width = nextFb->getDesc().width;
    viewportDesc.Height = nextFb->getDesc().height;
    _bloomCommandBuffer->setViewport(viewportDesc);
    {
      RenderPassBeginInfo rp{};
      rp.framebuffer = nextFb;
      _bloomCommandBuffer->beginRenderPass(rp);
    }
    _bloomCommandBuffer->bindResourceSet(_bloomUpSamplePassResourceSet, 0);
    _bloomCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
    _bloomCommandBuffer->draw(6, 0);
    _bloomCommandBuffer->endRenderPass();
  }

  // Reset viewport to window dimensions
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  _bloomCommandBuffer->setViewport(viewportDesc);

  // End command buffer and execute
  _bloomCommandBuffer->end();
  SubmitRecorded(renderDevice, _bloomCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[6].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::toneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice)
{
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // Begin command buffer recording
  _toneMappingCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set pipeline state
  _toneMappingCommandBuffer->bindGraphicsPipeline(_toneMappingPso);

  // Begin render pass (new API path)
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _toneMappingFb;
    rp.clearColors = {ClearColorValue{0, 0, 0, 1}};
    _toneMappingCommandBuffer->beginRenderPass(rp);
  }
  _toneMappingCommandBuffer->bindResourceSet(_toneMappingPassResourceSet, 0);
  _toneMappingCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  _toneMappingCommandBuffer->draw(6, 0);

  // End render pass and command buffer
  _toneMappingCommandBuffer->endRenderPass();
  _toneMappingCommandBuffer->end();
  SubmitRecorded(renderDevice, _toneMappingCommandBuffer);

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  _renderPassTimings[7].Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::debugPass(const std::shared_ptr<RenderDevice> &renderDevice,
                         const std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables,
                         const std::shared_ptr<CameraComponent> &camera)
{
  // Handle debug visualization based on display type
  switch (_debugDisplayType)
  {
  case DebugDisplayType::Disabled:
    renderToneMappedResult(renderDevice, camera);
    break;
  case DebugDisplayType::ShadowDepth:
    drawDebugRenderTarget(renderDevice, _shadowMapDepthTex, camera, false, true);
    break;
  case DebugDisplayType::Diffuse:
    drawDebugRenderTarget(renderDevice, _gBufferAlbedoTex, camera);
    break;
  case DebugDisplayType::Normal:
    drawDebugRenderTarget(renderDevice, _gBufferNormalTex, camera);
    break;
  case DebugDisplayType::Specular:
    drawDebugRenderTarget(renderDevice, _gBufferMaterialTex, camera);
    break;
  case DebugDisplayType::Depth:
    drawDebugRenderTarget(renderDevice, _gBufferDepthTex, camera);
    break;
  case DebugDisplayType::Lighting:
    drawDebugRenderTarget(renderDevice, _lightingColorTex, camera);
    break;
  case DebugDisplayType::Occulsion:
    drawDebugRenderTarget(renderDevice, _ssaoBlurTex, camera, true);
    break;
  case DebugDisplayType::Shadows:
    renderShadowDebugVisualization(renderDevice, camera);
    break;
  default:
    // Fallback to tone mapped result for unknown debug types
    renderToneMappedResult(renderDevice, camera);
    break;
  }

  // Always draw AABBs last to overlay on top of debug visualization
  drawAabb(renderDevice, aabbDrawables, camera);
}

void Renderer::renderToneMappedResult(const std::shared_ptr<RenderDevice> &renderDevice,
                                      const std::shared_ptr<CameraComponent> &camera)
{
  // Render the tone mapped result to the screen
  auto debugCommandBuffer = renderDevice->createCommandBuffer();
  debugCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);
  debugCommandBuffer->bindGraphicsPipeline(_editorDrawTexturedQuadPso);

  // Configure resource set and uniform buffer for final output
  TexturedQuadBuffer texturedQuadBufferData{};
  texturedQuadBufferData.NearClip = camera->getNear();
  texturedQuadBufferData.FarClip = camera->getFar();
  texturedQuadBufferData.SingleChannel = false;
  texturedQuadBufferData.ArraySlice = 0;
  texturedQuadBufferData.TextureArray = false;
  texturedQuadBufferData.OrthographicDepth = false;
  texturedQuadBufferData.PerspectiveDepth = false;
  texturedQuadBufferData.CubeArray = false;

  _debugPassResourceSet->reset();
  _debugPassResourceSet->addUniformBuffer(0, _fullscreenQuadBuffer);
  _debugPassResourceSet->addTexture(0, _toneMappingColorTex);
  _debugPassResourceSet->addTexture(1, getDefaultWhiteTexture());
  _debugPassResourceSet->addTexture(2, getDefaultWhiteTexture());
  _debugPassResourceSet->addSampler(0, _noMipSamplerState);
  _debugPassResourceSet->addSampler(1, _noMipSamplerState);
  _debugPassResourceSet->addSampler(2, _noMipSamplerState);

  _fullscreenQuadBuffer->writeData(0, sizeof(TexturedQuadBuffer), &texturedQuadBufferData, AccessType::WriteOnlyDiscard);
  _debugPassResourceSet->build(renderDevice);

  // Render to default framebuffer with viewport
  {
    RenderPassBeginInfo rp{};
    rp.clearColors = {ClearColorValue{0, 0, 0, 1}}; // clear to black
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    debugCommandBuffer->beginRenderPass(rp);
  }

  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  debugCommandBuffer->setViewport(viewportDesc);

  debugCommandBuffer->bindResourceSet(_debugPassResourceSet, 0);
  debugCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  debugCommandBuffer->draw(6, 1, 0, 0);
  debugCommandBuffer->endRenderPass();
  debugCommandBuffer->end();
  SubmitRecorded(renderDevice, debugCommandBuffer);
}

void Renderer::renderShadowDebugVisualization(const std::shared_ptr<RenderDevice> &renderDevice,
                                              const std::shared_ptr<CameraComponent> &camera)
{
  // Create command buffer for shadow debug visualization
  auto shadowDebugCommandBuffer = renderDevice->createCommandBuffer();
  shadowDebugCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  TexturedQuadBuffer texturedQuadBufferData{};
  texturedQuadBufferData.NearClip = camera->getNear();
  texturedQuadBufferData.FarClip = camera->getFar();
  texturedQuadBufferData.SingleChannel = true;
  texturedQuadBufferData.ArraySlice = _pointLightCubeMapToDraw;
  texturedQuadBufferData.TextureArray = false;
  texturedQuadBufferData.OrthographicDepth = false;
  texturedQuadBufferData.PerspectiveDepth = true;
  texturedQuadBufferData.CubeArray = true;

  shadowDebugCommandBuffer->bindGraphicsPipeline(_editorDrawTexturedQuadPso);

  _debugPassResourceSet->reset();
  _debugPassResourceSet->addUniformBuffer(0, _fullscreenQuadBuffer);
  _debugPassResourceSet->addTexture(0, getDefaultWhiteTexture());
  _debugPassResourceSet->addTexture(1, getDefaultWhiteTexture());
  _debugPassResourceSet->addTexture(2, _pointLightDepthTex);
  _debugPassResourceSet->addSampler(0, _noMipSamplerState);
  _debugPassResourceSet->addSampler(1, _noMipSamplerState);
  _debugPassResourceSet->addSampler(2, _noMipSamplerState);

  _fullscreenQuadBuffer->writeData(0, sizeof(TexturedQuadBuffer), &texturedQuadBufferData, AccessType::WriteOnlyDiscard);
  _debugPassResourceSet->build(renderDevice);

  // Begin render pass with clear
  {
    RenderPassBeginInfo rp{};
    rp.clearColors = {ClearColorValue{0, 0, 0, 1}};
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    shadowDebugCommandBuffer->beginRenderPass(rp);
  }

  // Set viewport
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  shadowDebugCommandBuffer->setViewport(viewportDesc);

  // Bind resource set and vertex buffer, then draw
  shadowDebugCommandBuffer->bindResourceSet(_debugPassResourceSet, 0);
  shadowDebugCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  shadowDebugCommandBuffer->draw(6, 1, 0, 0);

  shadowDebugCommandBuffer->endRenderPass();
  shadowDebugCommandBuffer->end();
  SubmitRecorded(renderDevice, shadowDebugCommandBuffer);
}

void Renderer::drawDrawable(const std::unique_ptr<ICommandBuffer> &commandBuffer,
                            const std::shared_ptr<RenderDevice> &renderDevice,
                            const std::shared_ptr<DrawableComponent> &drawable,
                            const std::shared_ptr<Material> &material,
                            const std::shared_ptr<CameraComponent> &camera)
{
  writePerObjectConstantData(drawable, material, camera);

  std::shared_ptr<StaticMesh> mesh = drawable->getMesh();
  commandBuffer->bindVertexBuffer(mesh->getVertexData(renderDevice));

  if (mesh->isIndexed())
  {
    auto indexCount = mesh->getIndexCount();
    commandBuffer->bindIndexBuffer(mesh->getIndexData(renderDevice));
    commandBuffer->drawIndexed(indexCount, 1, 0, 0, 0);
  }
  else
  {
    auto vertexCount = mesh->getVertexCount();
    commandBuffer->draw(vertexCount, 1, 0, 0);
  }
}

void Renderer::drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables,
                        const std::shared_ptr<CameraComponent> &camera)
{
  if (aabbDrawables.empty())
    return;

  // Copy depth buffer from G-Buffer to default framebuffer for depth testing using the new command buffer helper
  {
    auto copyCmd = renderDevice->createCommandBuffer();
    copyCmd->begin(CommandBufferUsage::OneTimeSubmit);
    copyCmd->blitDepthToDefault(_gBufferDepthTex);
    copyCmd->end();
    SubmitRecorded(renderDevice, copyCmd);
  }

  // Begin command buffer recording for AABB rendering
  auto aabbCommandBuffer = renderDevice->createCommandBuffer();
  aabbCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Begin render pass to default framebuffer (no clear - preserve existing content)
  {
    RenderPassBeginInfo rp{};
    // no clears; preserve existing content
    aabbCommandBuffer->beginRenderPass(rp);
  }

  // Set pipeline state and viewport
  aabbCommandBuffer->bindGraphicsPipeline(_drawAabbPso);
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  aabbCommandBuffer->setViewport(viewportDesc);

  // Bind the AABB vertex buffer once
  aabbCommandBuffer->bindVertexBuffer(_aabbVertexBuffer);

  // Draw each AABB
  for (auto drawable : aabbDrawables)
  {
    auto &aabb = drawable->getAabb();

    // Update per-object buffer for this AABB
    PerObjectBufferData objectBufferData;
    objectBufferData.Model = Matrix4::Translation(aabb.getCenter()) * Matrix4::Scaling(aabb.getExtents());
    objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
    objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
    _perObjectBuffer->writeData(0, sizeof(PerObjectBufferData), &objectBufferData, AccessType::WriteOnlyDiscard);

    // Reset and configure AABB pass resource set for this drawable
    _aabbPassResourceSet->reset();
    _aabbPassResourceSet->addUniformBuffer(0, _perObjectBuffer);
    _aabbPassResourceSet->build(renderDevice);

    // Bind resource set and draw
    aabbCommandBuffer->bindResourceSet(_aabbPassResourceSet, 0);
    aabbCommandBuffer->draw(AabbCoords.size(), 1, 0, 0);
  }

  // End render pass and command buffer
  aabbCommandBuffer->endRenderPass();
  aabbCommandBuffer->end();
  SubmitRecorded(renderDevice, aabbCommandBuffer);
}

void Renderer::drawDebugRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                                     std::shared_ptr<Texture> renderTarget,
                                     const std::shared_ptr<CameraComponent> &camera,
                                     bool singleChannel,
                                     bool orthographicDepth)
{
  TexturedQuadBuffer texturedQuadBufferData{};
  texturedQuadBufferData.NearClip = camera->getNear();
  texturedQuadBufferData.FarClip = camera->getFar();
  texturedQuadBufferData.SingleChannel = singleChannel;
  texturedQuadBufferData.ArraySlice = _shadowMapLayerToDraw;

  // Begin command buffer recording
  auto debugCommandBuffer = renderDevice->createCommandBuffer();
  debugCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set pipeline state
  debugCommandBuffer->bindGraphicsPipeline(_editorDrawTexturedQuadPso);

  // Reset and configure debug pass resource set
  _debugPassResourceSet->reset();
  _debugPassResourceSet->addUniformBuffer(0, _fullscreenQuadBuffer);

  if (renderTarget->getDesc().Usage == TextureUsage::RenderTarget)
  {
    texturedQuadBufferData.TextureArray = false;
    texturedQuadBufferData.OrthographicDepth = false;
    texturedQuadBufferData.PerspectiveDepth = false;
    texturedQuadBufferData.CubeArray = false;

    _debugPassResourceSet->addTexture(0, renderTarget);
    _debugPassResourceSet->addTexture(1, getDefaultWhiteTexture()); // Placeholder for TextureArray
    _debugPassResourceSet->addTexture(2, getDefaultWhiteTexture()); // Placeholder for TextureCubeArray
    _debugPassResourceSet->addSampler(0, _noMipSamplerState);
    _debugPassResourceSet->addSampler(1, _noMipSamplerState);
    _debugPassResourceSet->addSampler(2, _noMipSamplerState);
  }
  else if (renderTarget->getDesc().Usage == TextureUsage::Depth)
  {
    if (renderTarget->getTextureType() == TextureType::Texture2DArray)
    {
      texturedQuadBufferData.TextureArray = true;
      texturedQuadBufferData.OrthographicDepth = orthographicDepth;
      texturedQuadBufferData.PerspectiveDepth = !orthographicDepth;
      texturedQuadBufferData.CubeArray = false;

      _debugPassResourceSet->addTexture(0, getDefaultWhiteTexture()); // Placeholder for Texture
      _debugPassResourceSet->addTexture(1, renderTarget);
      _debugPassResourceSet->addTexture(2, getDefaultWhiteTexture()); // Placeholder for TextureCubeArray
      _debugPassResourceSet->addSampler(0, _noMipSamplerState);
      _debugPassResourceSet->addSampler(1, _noMipSamplerState);
      _debugPassResourceSet->addSampler(2, _noMipSamplerState);
    }
    else if (renderTarget->getTextureType() == TextureType::Texture2D)
    {
      texturedQuadBufferData.TextureArray = false;
      texturedQuadBufferData.OrthographicDepth = orthographicDepth;
      texturedQuadBufferData.PerspectiveDepth = !orthographicDepth;
      texturedQuadBufferData.CubeArray = false;

      _debugPassResourceSet->addTexture(0, renderTarget);
      _debugPassResourceSet->addTexture(1, getDefaultWhiteTexture()); // Placeholder for TextureArray
      _debugPassResourceSet->addTexture(2, getDefaultWhiteTexture()); // Placeholder for TextureCubeArray
      _debugPassResourceSet->addSampler(0, _noMipSamplerState);
      _debugPassResourceSet->addSampler(1, _noMipSamplerState);
      _debugPassResourceSet->addSampler(2, _noMipSamplerState);
    }
    else
    {
      // Unsupported texture type - clean up and return
      debugCommandBuffer->end();
      return;
    }
  }
  else
  {
    // Unsupported texture usage - clean up and return
    debugCommandBuffer->end();
    return;
  }

  // Write buffer data and build resource set
  _fullscreenQuadBuffer->writeData(0, sizeof(TexturedQuadBuffer), &texturedQuadBufferData, AccessType::WriteOnlyDiscard);
  _debugPassResourceSet->build(renderDevice);

  // Begin render pass to default framebuffer with clear
  {
    RenderPassBeginInfo rp{};
    rp.clearColors = {ClearColorValue{0, 0, 0, 1}};
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    debugCommandBuffer->beginRenderPass(rp);
  }

  // Set viewport
  ViewportDesc viewportDesc;
  viewportDesc.Width = _windowDims.X;
  viewportDesc.Height = _windowDims.Y;
  debugCommandBuffer->setViewport(viewportDesc);

  // Bind resource set and vertex buffer, then draw
  debugCommandBuffer->bindResourceSet(_debugPassResourceSet, 0);
  debugCommandBuffer->bindVertexBuffer(_fsQuadVertexBuffer);
  debugCommandBuffer->draw(6, 1, 0, 0);

  // End render pass and command buffer
  debugCommandBuffer->endRenderPass();
  debugCommandBuffer->end();
  SubmitRecorded(renderDevice, debugCommandBuffer);
}

std::vector<Matrix4> Renderer::calculateCameraCascadeProjections(const std::shared_ptr<CameraComponent> &camera) const
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

std::vector<Matrix4> Renderer::calculateCascadeLightTransforms(const std::shared_ptr<CameraComponent> &camera, const std::shared_ptr<LightComponent> &directionalLight) const
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
  shadowMapDesc.Count = _cascadeCount;
  _shadowMapDepthTex = renderDevice->createTexture(shadowMapDesc);
  {
    FramebufferDesc fb{};
    fb.width = _shadowMapResolution;
    fb.height = _shadowMapResolution;
    fb.samples = 1;
    fb.depthStencilAttachment = FramebufferAttachment{_shadowMapDepthTex};
    fb.hasDepthStencilAttachment = true;
    _shadowMapFb = std::make_shared<Framebuffer>(fb);
  }
  _shadowResolutionChanged = false;
}

void Renderer::writePerObjectConstantData(const std::shared_ptr<DrawableComponent> &drawable,
                                          const std::shared_ptr<Material> &material,
                                          const std::shared_ptr<CameraComponent> &camera) const
{
  PerObjectBufferData perObjectBufferData{};
  perObjectBufferData.Model = drawable->getWorldMatrix();
  perObjectBufferData.ModelView = camera->getView() * perObjectBufferData.Model;
  perObjectBufferData.ModelViewProjection = camera->getProj() * perObjectBufferData.ModelView;
  perObjectBufferData.DiffuseColour = material->getDiffuseColour();
  perObjectBufferData.DiffuseEnabled = material->diffuseTextureEnabled();
  perObjectBufferData.NormalEnabled = material->normalTextureEnabled();
  perObjectBufferData.MetalnessEnabled = material->metallicTextureEnabled();
  perObjectBufferData.RoughnessEnabled = material->roughnessTextureEnabled();
  perObjectBufferData.OcclusionEnabled = material->occlusionTextureEnabled();
  perObjectBufferData.OpacityEnabled = material->opacityTextureEnabled();
  perObjectBufferData.Metalness = material->getMetalness();
  perObjectBufferData.Roughness = material->getRoughness();

  _perObjectBuffer->writeData(0, sizeof(PerObjectBufferData), &perObjectBufferData, AccessType::WriteOnlyDiscard);
}

void Renderer::writePerFrameConstantData(const std::shared_ptr<CameraComponent> &camera,
                                         const std::shared_ptr<LightComponent> &directionalLight,
                                         const std::vector<std::shared_ptr<LightComponent>> &lights) const
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
  perFrameBufferData->ViewPosition = camera->getWorldPosition();
  perFrameBufferData->Exposure = _exposure;
  perFrameBufferData->ToneMappingEnabled = _toneMappingEnabled;
  perFrameBufferData->BloomStrength = _bloomStrength;
  perFrameBufferData->BloomThreshold = _bloomThreshold;
  perFrameBufferData->MaxPointLightShadowCasters = MAX_POINT_LIGHT_SHADOW_CASTERS;

  std::vector<LightData> lightDataArray;
  for (uint32 i = 0; i < lights.size(); i++)
  {
    const auto &light = lights[i];
    // TODO: Need to improve this as we only support one direction light.
    if (light->getLightType() != LightComponentType::Directional)
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
                                     const std::shared_ptr<CameraComponent> &camera) const
{
  // reset RNG to ensure identical sample kernel each update
  g_ssaoGenerator.seed(0);
  std::uniform_real_distribution<float32> randomFloats(0.0f, 1.0f);
  // reuse global deterministic RNG for sample kernel
  auto &generator = g_ssaoGenerator;
  std::vector<Vector3> ssaoKernel;
  ssaoKernel.reserve(_ssaoSamples);

  for (uint32 i = 0; i < _ssaoSamples; ++i)
  {
    Vector3 sample(randomFloats(generator, randomFloats.param()) * 2.0f - 1.0f,
                   randomFloats(generator, randomFloats.param()) * 2.0f - 1.0f,
                   randomFloats(generator, randomFloats.param()));

    sample.Normalize();
    sample *= randomFloats(generator, randomFloats.param());
    float32 scale = float32(i) / _ssaoSamples;

    // scale samples to be more centered around center of kernel
    scale = Math::Lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }

  // initialize struct and set only the used samples
  SsaoConstantsData ssaoConstantsData{};
  ssaoConstantsData.Bias = _ssaoBias;
  ssaoConstantsData.Radius = _ssaoRadius;
  ssaoConstantsData.KernelSize = _ssaoSamples;
  ssaoConstantsData.Intensity = _ssaoIntensity;
  for (uint32 i = 0; i < _ssaoSamples; ++i)
  {
    ssaoConstantsData.NoiseSamples[i] = ssaoKernel[i];
  }
  _ssaoConstantsBuffer->writeData(0, sizeof(SsaoConstantsData), &ssaoConstantsData, AccessType::WriteOnlyDiscard);
}

void Renderer::writePointLightConstantData(uint32 lightIndex, const Vector3 &position, float32 farPlane, const std::array<Matrix4, 6> &shadowMatrices) const
{
  PointLightBufferData pointLightBufferData{};
  pointLightBufferData.Position = position;
  pointLightBufferData.FarPlane = farPlane;
  pointLightBufferData.LightIndex = lightIndex;

  // Copy the 6 shadow matrices for the point light cubemap faces
  for (uint32 i = 0; i < 6; ++i)
  {
    pointLightBufferData.shadowMatrices[i] = shadowMatrices[i];
  }
  _pointLightBuffer->writeData(0, sizeof(PointLightBufferData), &pointLightBufferData, AccessType::WriteOnlyDiscard);
}

void Renderer::sortLightsForRendering(std::vector<std::shared_ptr<LightComponent>> &lights, const std::shared_ptr<CameraComponent> &camera) const
{
  const Vector3 cameraPosition = camera->getWorldPosition();

  // Sort lights with specific priority:
  // 1. Directional lights first (maintain their position)
  // 2. Point lights with shadows enabled, sorted by distance (closest first)
  // 3. Point lights without shadows, sorted by distance
  // 4. Other light types
  std::stable_sort(lights.begin(), lights.end(),
                   [&cameraPosition](const std::shared_ptr<LightComponent> &lightA, const std::shared_ptr<LightComponent> &lightB)
                   {
                     // Directional lights always come first
                     if (lightA->getLightType() == LightComponentType::Directional && lightB->getLightType() != LightComponentType::Directional)
                       return true;
                     if (lightB->getLightType() == LightComponentType::Directional && lightA->getLightType() != LightComponentType::Directional)
                       return false;

                     // If both are directional, maintain relative order
                     if (lightA->getLightType() == LightComponentType::Directional && lightB->getLightType() == LightComponentType::Directional)
                       return false;

                     // For point lights, prioritize shadow casters
                     bool aIsPointWithShadows = (lightA->getLightType() == LightComponentType::Point && lightA->getCastsShadows());
                     bool bIsPointWithShadows = (lightB->getLightType() == LightComponentType::Point && lightB->getCastsShadows());

                     if (aIsPointWithShadows && !bIsPointWithShadows)
                       return true;
                     if (bIsPointWithShadows && !aIsPointWithShadows)
                       return false;

                     // If both are point lights (with or without shadows), sort by distance
                     if (lightA->getLightType() == LightComponentType::Point && lightB->getLightType() == LightComponentType::Point)
                     {
                       Vector3 lightPosA = lightA->getPosition();
                       Vector3 lightPosB = lightB->getPosition();
                       float32 distanceA = (cameraPosition - lightPosA).Length();
                       float32 distanceB = (cameraPosition - lightPosB).Length();
                       return distanceA < distanceB;
                     }

                     // For other cases, maintain relative order
                     return false;
                   });
}

void Renderer::performFrustumCulling(const std::vector<std::shared_ptr<DrawableComponent>> &allDrawables,
                                     const std::shared_ptr<CameraComponent> &camera,
                                     std::vector<std::shared_ptr<DrawableComponent>> &opaqueDrawables,
                                     std::vector<std::shared_ptr<DrawableComponent>> &transparentDrawables,
                                     std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables)
{
  // Clear cached vectors and reserve capacity to avoid reallocations
  _cachedOpaqueDrawables.clear();
  _cachedTransparentDrawables.clear();
  _cachedAabbDrawables.clear();

  // Reserve capacity based on typical scene composition (avoid repeated reallocations)
  _cachedOpaqueDrawables.reserve(allDrawables.size() * 3 / 4);  // Estimate 75% opaque
  _cachedTransparentDrawables.reserve(allDrawables.size() / 4); // Estimate 25% transparent
  _cachedAabbDrawables.reserve(allDrawables.size() / 10);       // Estimate 10% with debug AABB

  for (const auto &drawable : allDrawables)
  {
    // PERFORMANCE OPTIMIZATION: Use cached transform instead of creating new Transform every frame
    const TransformComponent *transform = drawable->getTransform();
    if (transform && camera->contains(drawable->getAabb(), Matrix4::Identity))
    {
      // If the drawable is not visible, skip it
      if (!drawable->isVisible())
        continue;

      // Direct classification without double processing through RenderQueue
      if (drawable->getMaterial()->hasOpacityTexture())
      {
        _cachedTransparentDrawables.push_back(drawable);
      }
      else
      {
        _cachedOpaqueDrawables.push_back(drawable);
      }

      if (drawable->shouldDrawAabb())
      {
        _cachedAabbDrawables.push_back(drawable);
      }
    }
    // If the drawable is not in the camera's frustum, skip it (continue)
  }

  // Efficient sorting using cached distance calculation to avoid repeated computations
  const Vector3 cameraPos = camera->getWorldPosition();

  // Sort opaque objects front-to-back for better z-culling
  std::sort(_cachedOpaqueDrawables.begin(), _cachedOpaqueDrawables.end(),
            [&cameraPos](const auto &a, const auto &b)
            {
              // Calculate distance once and cache
              const auto *transformA = a->getTransform();
              const auto *transformB = b->getTransform();
              if (!transformA || !transformB)
                return false;

              Vector3 posA = transformA->getPosition();
              Vector3 posB = transformB->getPosition();
              float distA = (cameraPos - posA).Length();
              float distB = (cameraPos - posB).Length();
              return distA < distB;
            });

  // Sort transparent objects back-to-front for proper alpha blending
  std::sort(_cachedTransparentDrawables.begin(), _cachedTransparentDrawables.end(),
            [&cameraPos](const auto &a, const auto &b)
            {
              const auto *transformA = a->getTransform();
              const auto *transformB = b->getTransform();
              if (!transformA || !transformB)
                return false;

              Vector3 posA = transformA->getPosition();
              Vector3 posB = transformB->getPosition();
              float distA = (cameraPos - posA).Length();
              float distB = (cameraPos - posB).Length();
              return distA > distB; // Note: reversed for back-to-front
            });

  // Move results efficiently using move semantics
  opaqueDrawables = std::move(_cachedOpaqueDrawables);
  transparentDrawables = std::move(_cachedTransparentDrawables);
  aabbDrawables = std::move(_cachedAabbDrawables);
}

std::shared_ptr<Texture> Renderer::getDefaultWhiteTexture() const
{
  return _defaultWhiteTexture;
}

std::shared_ptr<Texture> Renderer::getDefaultNormalTexture() const
{
  return _defaultNormalTexture;
}

void Renderer::pointLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                   const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                                   const std::vector<std::shared_ptr<LightComponent>> &lights,
                                   const std::shared_ptr<CameraComponent> &camera)
{
  // Begin command buffer recording
  _pointLightDepthCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Set viewport
  ViewportDesc viewportDesc;
  viewportDesc.Height = _pointLightShadowMapResolution;
  viewportDesc.Width = _pointLightShadowMapResolution;
  _pointLightDepthCommandBuffer->setViewport(viewportDesc);

  // Set pipeline state
  _pointLightDepthCommandBuffer->bindGraphicsPipeline(_pointLightDepthPso);

  // Begin render pass
  {
    RenderPassBeginInfo rp{};
    rp.framebuffer = _pointLightDepthFb;
    // depth-only clear for cubemap array shadow pass
    rp.clearDepthStencil = std::make_unique<ClearDepthStencilValue>(ClearDepthStencilValue{1.0f, 0});
    _pointLightDepthCommandBuffer->beginRenderPass(rp);
  }
  _pointLightDepthCommandBuffer->bindResourceSet(_pointLightDepthPassResourceSet, 0);

  // Lights are already sorted by sortLightsForRendering(), so process sequentially
  // Track the index in the filtered non-directional lights array for shader consistency
  uint32 filteredLightIndex = 0; // Index in Constants.Lights[] array
  int shadowCasterCount = 0;

  for (uint32 i = 0; i < lights.size(); ++i)
  {
    const auto &light = lights[i];

    // Skip directional lights
    if (light->getLightType() == LightComponentType::Directional)
      continue;

    // For point lights that cast shadows
    if (light->getLightType() == LightComponentType::Point)
    {
      if (shadowCasterCount >= MAX_POINT_LIGHT_SHADOW_CASTERS)
      {
        filteredLightIndex++; // Still increment to maintain index consistency
        continue;
      }

      // Skip lights that don't cast shadows
      if (!light->getCastsShadows())
      {
        filteredLightIndex++; // Still increment to maintain index consistency
        continue;
      }

      // Compute six 90° view-proj matrices for this point light
      Vector3 pos = light->getPosition();
      float nearPlane = 0.1f;
      float farPlane = light->getRadius();
      const std::array<Vector3, 6> dirs = {{Vector3(1, 0, 0), Vector3(-1, 0, 0),
                                            Vector3(0, 1, 0), Vector3(0, -1, 0),
                                            Vector3(0, 0, 1), Vector3(0, 0, -1)}};
      const std::array<Vector3, 6> ups = {{Vector3(0, -1, 0), Vector3(0, -1, 0),
                                           Vector3(0, 0, 1), Vector3(0, 0, -1),
                                           Vector3(0, -1, 0), Vector3(0, -1, 0)}};
      std::array<Matrix4, 6> shadowMatrices;
      Matrix4 proj = Matrix4::Perspective(Degree(90.0f), 1.0f, nearPlane, farPlane);
      for (int j = 0; j < 6; ++j)
      {
        Matrix4 view = Matrix4::LookAt(pos, pos + dirs[j], ups[j]);
        shadowMatrices[j] = proj * view;
      }

      // Use filteredLightIndex to match Constants.Lights[] array indexing
      writePointLightConstantData(filteredLightIndex, pos, farPlane, shadowMatrices);
      shadowCasterCount++;

      // Check culling settings and only perform necessary culling work
      const auto &cullingSettings = _pointLightCuller->getCullingSettings();

      // Always use the PointLightCuller for consistent culling and statistics
      auto cullingResult = _pointLightCuller->cullObjectsForPointLight(light, drawables);

      // Store per-light culling statistics
      light->setCullingStats(&cullingResult);

      // Get final objects to render (deduplicated from all faces)
      std::vector<std::shared_ptr<DrawableComponent>> finalObjects;
      if (cullingSettings.enableFaceCulling)
      {
        // Combine all face-culled objects and deduplicate
        for (int face = 0; face < 6; ++face)
        {
          const auto &faceObjects = cullingResult.faceCulled[face];
          finalObjects.insert(finalObjects.end(), faceObjects.begin(), faceObjects.end());
        }

        // Remove duplicates since objects can appear in multiple faces
        std::sort(finalObjects.begin(), finalObjects.end());
        finalObjects.erase(std::unique(finalObjects.begin(), finalObjects.end()), finalObjects.end());
      }
      else
      {
        // No face culling - use sphere culled objects directly
        finalObjects = cullingResult.sphereCulled;
      }

      // Render the final culled objects using command buffer
      for (const auto &drawable : finalObjects)
      {
        std::shared_ptr<Material> material(drawable->getMaterial());
        drawDrawable(_pointLightDepthCommandBuffer, renderDevice, drawable, material, camera);
      }
    }

    // Increment the filtered light index for all non-directional lights
    filteredLightIndex++;

    // Handle other light types (spots, etc.) here if needed
    // For now, just increment the counter to maintain consistency
  }

  // End render pass and command buffer
  _pointLightDepthCommandBuffer->endRenderPass();
  _pointLightDepthCommandBuffer->end();
  SubmitRecorded(renderDevice, _pointLightDepthCommandBuffer);
}
