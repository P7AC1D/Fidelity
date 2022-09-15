#pragma once
#include <vector>

#include "../Core/Maths.h"
#include "Renderer.h"

class Camera;
class Drawable;
class GpuBuffer;
class Light;
class PipelineState;
class RenderTarget;
class SamplerState;
class Texture;

class ShadowMapRenderer : public Renderer
{
public:
  ShadowMapRenderer();

  void onInit(const std::shared_ptr<RenderDevice> &renderDevice) override;
  void onDrawDebugUi() override;

  void drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                 const std::vector<std::shared_ptr<Light>> &lights,
                 const std::shared_ptr<Camera> &camera);

  std::shared_ptr<RenderTarget> getShadowMapRto() { return _shadowMapRto; }
  std::shared_ptr<GpuBuffer> getShadowBuffer() { return _shadowBufer; }

private:
  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const;

  float32 _zMulti;
  int32 _shadowMapResolution;
  uint32 _cascadeCount;

  std::shared_ptr<Texture> _shadowCubeMap;
  std::shared_ptr<RenderTarget> _shadowMapRto;
  std::shared_ptr<PipelineState> _shadowMapPso;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _transformsBuffer;
  std::shared_ptr<GpuBuffer> _shadowBufer;
};