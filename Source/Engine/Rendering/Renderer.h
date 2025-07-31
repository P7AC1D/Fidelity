#pragma once
#include <memory>
#include <string>
#include <vector>

#include "DrawableComponent.h"
#include "LightComponent.h"
#include "CameraComponent.h"

#include "../Core/Maths.h"
#include "../Core/Types.hpp"
#include "../RenderApi/ResourceSet.hpp"

class GpuBuffer;
class Material;
class PipelineState;
class RenderDevice;
class RenderQueue;
class RenderTarget;
class SamplerState;
class Texture;
class VertexBuffer;

// Forward declarations that need full definitions for unique_ptr
#include "ShadowFrustum.h"
#include "RenderQueue.h"
#include "PointLightCuller.h"

struct RenderPassTimings
{
  uint64 Duration;
  std::string Name;
};

enum class DebugDisplayType
{
  Disabled,
  ShadowDepth,
  Diffuse,
  Normal,
  Specular,
  Depth,
  Shadows,
  Lighting,
  Occulsion,
};

class Renderer
{
public:
  Renderer(const Vector2I &windowDims);

  bool init(const std::shared_ptr<RenderDevice> &renderDevice);
  void drawDebugUi();

  void drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                 const std::vector<std::shared_ptr<DrawableComponent>> &allDrawables,
                 const std::vector<std::shared_ptr<LightComponent>> &lights,
                 const std::shared_ptr<CameraComponent> &camera);

  const std::vector<RenderPassTimings> &getRenderPassTimings() const { return _renderPassTimings; }

private:
  std::unique_ptr<IResourceSet> createMaterialResourceSet(const std::shared_ptr<RenderDevice> &renderDevice,
                                                          const std::shared_ptr<Material> &material);

  void initConstantBuffers(const std::shared_ptr<RenderDevice> &renderDevice);
  void initSamplers(const std::shared_ptr<RenderDevice> &renderDevice);
  void initTextures(const std::shared_ptr<RenderDevice> &renderDevice);
  void initResourceSets(const std::shared_ptr<RenderDevice> &renderDevice);
  void updateResourceSets(const std::shared_ptr<RenderDevice> &renderDevice);

  void initDirectionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initPointLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initGbufferPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initTransparencyPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initSsaoPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initLightingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initBloomDownSamplePass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initBloomUpSamplePass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initToneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initDebugPass(const std::shared_ptr<RenderDevice> &renderDevice);

  void directionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                 const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                                 const std::shared_ptr<LightComponent> &directionalLight,
                                 const std::shared_ptr<CameraComponent> &camera);
  void pointLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                           const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                           const std::vector<std::shared_ptr<LightComponent>> &lights,
                           const std::shared_ptr<CameraComponent> &camera);

  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<DrawableComponent>> &drawables,
                   const std::shared_ptr<CameraComponent> &camera);
  void transparencyPass(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::vector<std::shared_ptr<DrawableComponent>> &transparentDrawables,
                        const std::shared_ptr<CameraComponent> &camera);
  void ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::shared_ptr<CameraComponent> &camera);
  void lightingPass(const std::shared_ptr<RenderDevice> &renderDevice,
                    const std::vector<std::shared_ptr<LightComponent>> &lights,
                    const std::shared_ptr<CameraComponent> &camera);
  void bloomPass(const std::shared_ptr<RenderDevice> &rendereDevice);
  void toneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void debugPass(const std::shared_ptr<RenderDevice> &renderDevice,
                 const std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables,
                 const std::shared_ptr<CameraComponent> &camera);

  void drawDrawable(const std::shared_ptr<RenderDevice> &renderDevice,
                    const std::shared_ptr<DrawableComponent> &drawable,
                    const std::shared_ptr<Material> &material,
                    const std::shared_ptr<CameraComponent> &camera);

  void drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables,
                const std::shared_ptr<CameraComponent> &camera);

  void drawDebugRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                             std::shared_ptr<Texture> renderTarget,
                             const std::shared_ptr<CameraComponent> &camera,
                             bool singleChannel = false,
                             bool orthographicDepth = false);

  std::vector<Matrix4> calculateCameraCascadeProjections(const std::shared_ptr<CameraComponent> &camera) const;
  std::vector<float32> calculateCascadeLevels(float32 nearClip, float32 farClip) const;
  std::vector<Matrix4> calculateCascadeLightTransforms(const std::shared_ptr<CameraComponent> &camera, const std::shared_ptr<LightComponent> &directionalLight) const;

  void createDirectionalLightShadowDepthMap(const std::shared_ptr<RenderDevice> &renderDevice);

  void writePerObjectConstantData(const std::shared_ptr<DrawableComponent> &drawable,
                                  const std::shared_ptr<Material> &material,
                                  const std::shared_ptr<CameraComponent> &camera) const;
  void writePerFrameConstantData(const std::shared_ptr<CameraComponent> &camera,
                                 const std::shared_ptr<LightComponent> &directionalLight,
                                 const std::vector<std::shared_ptr<LightComponent>> &lights) const;
  void writeSsaoConstantData(const std::shared_ptr<RenderDevice> &renderDevice, const std::shared_ptr<CameraComponent> &camera) const;
  void writePointLightConstantData(uint32 lightIndex, const Vector3 &position, float32 farPlane, const std::array<Matrix4, 6> &shadowMatrices) const;

  // Light sorting for consistent shadow and lighting passes
  void sortLightsForRendering(std::vector<std::shared_ptr<LightComponent>> &lights, const std::shared_ptr<CameraComponent> &camera) const;

  // Frustum culling and object categorization
  void performFrustumCulling(const std::vector<std::shared_ptr<DrawableComponent>> &allDrawables,
                             const std::shared_ptr<CameraComponent> &camera,
                             std::vector<std::shared_ptr<DrawableComponent>> &opaqueDrawables,
                             std::vector<std::shared_ptr<DrawableComponent>> &transparentDrawables,
                             std::vector<std::shared_ptr<DrawableComponent>> &aabbDrawables);

  std::shared_ptr<Texture> getDefaultWhiteTexture() const;
  std::shared_ptr<Texture> getDefaultNormalTexture() const;

  // Default textures
  std::shared_ptr<Texture> _defaultWhiteTexture;
  std::shared_ptr<Texture> _defaultNormalTexture;

  Vector2I _windowDims;
  Colour _ambientColour;
  float32 _ambientIntensity;

  uint32 _ssaoSamples;
  float32 _ssaoBias;
  float32 _ssaoRadius;
  float32 _ssaoIntensity;
  bool _ssaoEnabled;
  bool _ssaoSettingsModified;
  bool _settingsModified;

  // ----- Shadow settings -----
  bool _drawCascadeLayers;
  bool _shadowResolutionChanged;
  int32 _shadowMapResolution;
  uint32 _pointLightShadowMapResolution;
  uint32 _cascadeCount;
  uint32 _shadowSampleCount;
  float32 _shadowSampleSpread;
  float32 _minCascadeDistance, _maxCascadeDistance;
  float32 _cascadeLambda;
  // ----- HDR settings -----
  bool _toneMappingEnabled;
  bool _bloomEnabled;
  float32 _exposure;
  float32 _bloomStrength;
  float32 _bloomFilter;
  float32 _bloomThreshold;

  // ----- Editor settings -----
  DebugDisplayType _debugDisplayType;
  int32 _shadowMapLayerToDraw;
  int32 _pointLightCubeMapToDraw;

  std::vector<RenderPassTimings> _renderPassTimings;

  std::shared_ptr<GpuBuffer> _perObjectBuffer,
      _perFrameBuffer,
      _ssaoConstantsBuffer,
      _fullscreenQuadBuffer,
      _bloomBuffer,
      _pointLightBuffer;
  std::shared_ptr<RenderTarget> _shadowMapRto,
      _pointLightDepthRto,
      _gBufferRto,
      _transparencyRto,
      _ssaoRto,
      _ssaoBlurRto,
      _lightingPassRto,
      _toneMappingRto;
  std::vector<std::shared_ptr<RenderTarget>> _bloomDownSampleRtos;
  std::shared_ptr<PipelineState> _shadowMapPso,
      _pointLightDepthPso,
      _gBufferPso,
      _transparencyPso,
      _ssaoPso,
      _ssaoBlurPso,
      _lightingPso,
      _bloomDownSamplePso,
      _bloomUpSamplePso,
      _toneMappingPso,
      _drawAabbPso,
      _editorDrawTexturedQuadPso;
  std::shared_ptr<SamplerState> _basicSamplerState,
      _noMipSamplerState,
      _shadowMapSamplerState,
      _ssaoNoiseSampler,
      _noMipWithBorderSamplerState,
      _bloomSamplerState,
      _linearNoMipSamplerState;
  std::shared_ptr<VertexBuffer> _fsQuadVertexBuffer,
      _aabbVertexBuffer;
  std::shared_ptr<Texture> _randomRotationsMap,
      _ssaoNoiseTexture;

  // Render queues for culling and sorting
  std::unique_ptr<RenderQueue> _opaqueQueue;
  std::unique_ptr<RenderQueue> _transparentQueue;

  // Shadow culling system
  std::unique_ptr<ShadowFrustum> _shadowFrustum;
  std::unique_ptr<RenderQueue> _shadowQueue;

  // Point light culling system
  std::unique_ptr<PointLightCuller> _pointLightCuller;
  PointLightCuller::CullingSettings _pointLightCullingSettings;

  // Optimized frustum culling vectors - cached to avoid repeated allocations
  std::vector<std::shared_ptr<DrawableComponent>> _cachedOpaqueDrawables;
  std::vector<std::shared_ptr<DrawableComponent>> _cachedTransparentDrawables;
  std::vector<std::shared_ptr<DrawableComponent>> _cachedAabbDrawables;

  // Resource Set Layouts
  std::unique_ptr<IResourceSetLayout> _shadowPassLayout;
  std::unique_ptr<IResourceSetLayout> _pointLightDepthPassLayout;
  std::unique_ptr<IResourceSetLayout> _gbufferPassLayout;
  std::unique_ptr<IResourceSetLayout> _ssaoPassLayout;
  std::unique_ptr<IResourceSetLayout> _ssaoBlurPassLayout;
  std::unique_ptr<IResourceSetLayout> _lightingPassLayout;
  std::unique_ptr<IResourceSetLayout> _bloomDownSamplePassLayout;
  std::unique_ptr<IResourceSetLayout> _bloomUpSamplePassLayout;
  std::unique_ptr<IResourceSetLayout> _toneMappingPassLayout;
  std::unique_ptr<IResourceSetLayout> _materialLayout;
  std::unique_ptr<IResourceSetLayout> _debugPassLayout;
  std::unique_ptr<IResourceSetLayout> _aabbPassLayout;

  // Resource Sets
  std::unique_ptr<IResourceSet> _shadowPassResourceSet;
  std::unique_ptr<IResourceSet> _pointLightDepthPassResourceSet;
  std::unique_ptr<IResourceSet> _gbufferPassResourceSet;
  std::unique_ptr<IResourceSet> _ssaoPassResourceSet;
  std::unique_ptr<IResourceSet> _ssaoBlurPassResourceSet;
  std::unique_ptr<IResourceSet> _lightingPassResourceSet;
  std::unique_ptr<IResourceSet> _bloomDownSamplePassResourceSet;
  std::unique_ptr<IResourceSet> _bloomUpSamplePassResourceSet;
  std::unique_ptr<IResourceSet> _toneMappingPassResourceSet;
  std::unique_ptr<IResourceSet> _materialResourceSet;
  std::unique_ptr<IResourceSet> _debugPassResourceSet;
  std::unique_ptr<IResourceSet> _aabbPassResourceSet;
};
