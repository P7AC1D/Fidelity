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
                 const Camera &camera);

  std::shared_ptr<RenderTarget> getGbuffer() { return _gBufferRto; }
  std::shared_ptr<RenderTarget> getLightPrepassBuffer() { return _lightPrePassRto; }
  std::shared_ptr<RenderTarget> getMergePassBuffer() { return _mergePassRto; }

private:
  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                   const Camera &camera);

  void lightPrePass(std::shared_ptr<RenderDevice> renderDevice,
                    const std::vector<std::shared_ptr<Light>> &lights,
                    const Camera &camera);

  void mergePass(std::shared_ptr<RenderDevice> renderDevice);

  void writeMaterialConstantData(std::shared_ptr<RenderDevice> renderDevice,
                                 std::shared_ptr<Material> material) const;
  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const Camera &camera) const;

  Vector2I _windowDims;

  std::shared_ptr<PipelineState> _mergePso;
  std::shared_ptr<PipelineState> _lightPrePassCWPto, _lightPrePassCCWPto;
  std::shared_ptr<PipelineState> _gBufferPso;
  std::shared_ptr<RenderTarget> _lightPrePassRto;
  std::shared_ptr<RenderTarget> _mergePassRto;
  std::shared_ptr<RenderTarget> _gBufferRto;
  std::shared_ptr<GpuBuffer> _materialBuffer;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _pointLightConstantsBuffer;
  std::shared_ptr<GpuBuffer> _pointLightBuffer;
  std::shared_ptr<StaticMesh> _pointLightMesh;
};