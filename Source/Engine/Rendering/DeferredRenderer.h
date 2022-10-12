#pragma once
#include <memory>
#include <vector>

#include "../Core/Maths.h"
#include "Renderer.h"

class Camera;
class Drawable;
class GpuBuffer;
class Material;
class Light;
class PipelineState;
class RenderDevice;
class RenderTarget;
class SamplerState;
class StaticMesh;
class Texture;
class VertexBuffer;

class DeferredRenderer : public Renderer
{
public:
  DeferredRenderer(const Vector2I &windowDims);

  void onInit(const std::shared_ptr<RenderDevice> &renderDevice) override;
  void onDrawDebugUi() override;

  // TODO: Removed shared_ptr here as params as there is no ownership occuring internally. Objects are expected to live for the duration of the call.
  void drawFrame(std::shared_ptr<RenderDevice> renderDevice,
                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                 const std::vector<std::shared_ptr<Light>> &lights,
                 const std::shared_ptr<RenderTarget> &shadowMapRto,
                 const std::shared_ptr<GpuBuffer> &shadowMapBuffer,
                 const std::shared_ptr<Camera> &camera);

  std::shared_ptr<RenderTarget> getGbuffer() { return _gBufferRto; }
  std::shared_ptr<RenderTarget> getShadowRto() { return _shadowsRto; }
  std::shared_ptr<RenderTarget> getSsaoRto() { return _ssaoBlurRto; }
  std::shared_ptr<RenderTarget> getLightingBuffer() { return _lightingPassRto; }

private:
  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                   const std::shared_ptr<Camera> &camera);

  void shadowPass(const std::shared_ptr<RenderDevice> &renderDevice,
                  const std::shared_ptr<RenderTarget> &shadowMapRto,
                  const std::shared_ptr<GpuBuffer> &shadowMapBuffer);

  void ssaoPass(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::shared_ptr<Camera> &camera);
  void ssaoBlurPass(const std::shared_ptr<RenderDevice> &renderDevice,
                    const std::shared_ptr<Camera> &camera);

  void lightingPass(std::shared_ptr<RenderDevice> renderDevice,
                    const std::vector<std::shared_ptr<Light>> &lights,
                    const std::shared_ptr<RenderTarget> &shadowMapRto,
                    const std::shared_ptr<GpuBuffer> &shadowMapBuffer,
                    const std::shared_ptr<Camera> &camera);

  void writeMaterialConstantData(std::shared_ptr<RenderDevice> renderDevice,
                                 std::shared_ptr<Material> material) const;
  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const;
  void writeSsaoNoiseTexture(const std::shared_ptr<RenderDevice> &renderDevice) const;
  void writeSsaoConstantData(const std::shared_ptr<RenderDevice> &renderDevice,
                             const std::shared_ptr<Camera> &camera) const;

  Vector2I _windowDims;
  Colour _ambientColour;
  float32 _ambientIntensity;

  uint32 _ssaoSamples;
  float32 _ssaoBias;
  float32 _ssaoRadius;
  bool _ssaoEnabled;

  std::shared_ptr<PipelineState> _gBufferPso, _shadowsPso, _ssaoPso, _ssaoBlurPso, _lightingPto;
  std::shared_ptr<RenderTarget> _gBufferRto, _shadowsRto, _ssaoRto, _ssaoBlurRto, _lightingPassRto;
  std::shared_ptr<GpuBuffer> _materialBuffer;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _lightingConstantsBuffer;
  std::shared_ptr<GpuBuffer> _lightingBuffer, _ssaoConstantsBuffer;
  std::shared_ptr<SamplerState> _shadowMapSamplerState, _ssaoNoiseSampler;
  std::shared_ptr<Texture> _randomRotationsMap, _ssaoNoiseTexture;
};