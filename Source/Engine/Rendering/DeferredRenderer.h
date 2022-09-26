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
  std::shared_ptr<RenderTarget> getLightingBuffer() { return _lightingPassRto; }

private:
  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                   const std::shared_ptr<Camera> &camera);

  void lightingPass(std::shared_ptr<RenderDevice> renderDevice,
                    const std::vector<std::shared_ptr<Light>> &lights,
                    const std::shared_ptr<RenderTarget> &shadowMapRto,
                    const std::shared_ptr<GpuBuffer> &shadowMapBuffer,
                    const std::shared_ptr<Camera> &camera);

  void writeMaterialConstantData(std::shared_ptr<RenderDevice> renderDevice,
                                 std::shared_ptr<Material> material) const;
  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const;

  Vector2I _windowDims;
  Colour _ambientColour;
  float32 _ambientIntensity;

  std::shared_ptr<PipelineState> _mergePso;
  std::shared_ptr<PipelineState> _gBufferPso;
  std::shared_ptr<PipelineState> _lightingPto;
  std::shared_ptr<RenderTarget> _lightingPassRto;
  std::shared_ptr<RenderTarget> _gBufferRto;
  std::shared_ptr<GpuBuffer> _materialBuffer;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _lightingConstantsBuffer;
  std::shared_ptr<GpuBuffer> _lightingBuffer;
  std::shared_ptr<SamplerState> _shadowMapSamplerState;
};