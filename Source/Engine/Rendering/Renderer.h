#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Core/Maths.h"
#include "../Core/Types.hpp"

class Drawable;
class GpuBuffer;
class Light;
class Material;
class PipelineState;
class RenderDevice;
class RenderTarget;
class SamplerState;
class Texture;
class VertexBuffer;

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
                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                 const std::vector<std::shared_ptr<Drawable>> &opaqueDrawables,
                 const std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                 const std::vector<std::shared_ptr<Drawable>> &allDrawables,
                 const std::vector<std::shared_ptr<Light>> &lights,
                 const std::shared_ptr<Camera> &camera);

  const std::vector<RenderPassTimings> &getRenderPassTimings() const { return _renderPassTimings; }

private:
  void initConstantBuffers(const std::shared_ptr<RenderDevice> &renderDevice);
  void initSamplers(const std::shared_ptr<RenderDevice> &renderDevice);
  void initTextures(const std::shared_ptr<RenderDevice> &renderDevice);

  void initDirectionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initGbufferPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initTransparencyPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initShadowPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initSsaoPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initLightingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initBloomDownSamplePass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initBloomUpSamplePass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initToneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void initDebugPass(const std::shared_ptr<RenderDevice> &renderDevice);

  void directionalLightDepthPass(const std::shared_ptr<RenderDevice> &renderDevice,
                                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                                 const std::shared_ptr<Light> &directionalLight,
                                 const std::shared_ptr<Camera> &camera);
  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                   const std::shared_ptr<Camera> &camera);
  void transparencyPass(const std::shared_ptr<RenderDevice> &renderDevice,
                        const std::vector<std::shared_ptr<Drawable>> &transparentDrawables,
                        const std::shared_ptr<Camera> &camera);
  void shadowPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::shared_ptr<Camera> &camera);
  void lightingPass(const std::shared_ptr<RenderDevice> &renderDevice,
                    const std::vector<std::shared_ptr<Light>> &lights,
                    const std::shared_ptr<Camera> &camera);
  void bloomPass(const std::shared_ptr<RenderDevice> &rendereDevice);
  void toneMappingPass(const std::shared_ptr<RenderDevice> &renderDevice);
  void debugPass(const std::shared_ptr<RenderDevice> &renderDevice,
                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                 const std::shared_ptr<Camera> &camera);

  void drawDrawable(const std::shared_ptr<RenderDevice> &renderDevice,
                    const std::shared_ptr<Drawable> &drawable,
                    const std::shared_ptr<Material> &material,
                    const std::shared_ptr<Camera> &camera);

  void drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                const std::shared_ptr<Camera> &camera);

  void drawDebugRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                             std::shared_ptr<Texture> renderTarget,
                             const std::shared_ptr<Camera> &camera,
                             bool singleChannel = false,
                             bool orthographicDepth = false);

  std::vector<Matrix4> calculateCameraCascadeProjections(const std::shared_ptr<Camera> &camera) const;
  std::vector<float32> calculateCascadeLevels(float32 nearClip, float32 farClip) const;
  std::vector<Matrix4> calculateCascadeLightTransforms(const std::shared_ptr<Camera> &camera, const std::shared_ptr<Light> &directionalLight) const;

  void createDirectionalLightShadowDepthMap(const std::shared_ptr<RenderDevice> &renderDevice);

  void writePerObjectConstantData(const std::shared_ptr<Drawable> &drawable,
                                  const std::shared_ptr<Material> &material,
                                  const std::shared_ptr<Camera> &camera) const;
  void writePerFrameConstantData(const std::shared_ptr<Camera> &camera,
                                 const std::shared_ptr<Light> &directionalLight,
                                 const std::vector<std::shared_ptr<Light>> &lights) const;
  void writeSsaoConstantData(const std::shared_ptr<RenderDevice> &renderDevice, const std::shared_ptr<Camera> &camera) const;

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

  std::vector<RenderPassTimings> _renderPassTimings;

  std::shared_ptr<GpuBuffer> _perObjectBuffer,
      _perFrameBuffer,
      _ssaoConstantsBuffer,
      _fullscreenQuadBuffer,
      _bloomBuffer;
  std::shared_ptr<RenderTarget> _shadowMapRto,
      _gBufferRto,
      _transparencyRto,
      _shadowsRto,
      _ssaoRto,
      _ssaoBlurRto,
      _lightingPassRto,
      _toneMappingRto;
  std::vector<std::shared_ptr<RenderTarget>> _bloomDownSampleRtos;
  std::shared_ptr<PipelineState> _shadowMapPso,
      _gBufferPso,
      _transparencyPso,
      _shadowsPso,
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
      _bloomSamplerState;
  std::shared_ptr<VertexBuffer> _fsQuadVertexBuffer,
      _aabbVertexBuffer;
  std::shared_ptr<Texture> _randomRotationsMap,
      _ssaoNoiseTexture;
};