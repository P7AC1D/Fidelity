#pragma once

#include <memory>
#include <vector>

#include "Renderer.h"

class Camera;
class Drawable;
class GpuBuffer;
class PipelineState;
class RenderDevice;
class RenderTarget;
class SamplerState;
class Texture;
class VertexBuffer;

enum class DebugDisplayType
{
  Disabled,
  ShadowMap,
  Diffuse,
  Normal,
  Depth
};

class DebugRenderer : public Renderer
{
public:
  DebugRenderer();

  void onInit(const std::shared_ptr<RenderDevice> &renderDevice) override;
  void onDrawDebugUi() override;

  void drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                 const std::shared_ptr<RenderTarget> &gBuffer,
                 const std::shared_ptr<RenderTarget> &lightingBuffer,
                 const std::shared_ptr<RenderTarget> &shadowMapBuffer,
                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                 const std::shared_ptr<Camera> &camera);

private:
  void drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                const std::shared_ptr<Camera> &camera);

  void drawRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                        std::shared_ptr<Texture> renderTarget,
                        const std::shared_ptr<Camera> &camera);

  DebugDisplayType _debugDisplayType;
  int32 _shadowMapLayerToDraw;

  std::shared_ptr<PipelineState> _gbufferDebugDrawPso;
  std::shared_ptr<PipelineState> _depthDebugDrawPso;
  std::shared_ptr<PipelineState> _shadowMapDebugPso;
  std::shared_ptr<PipelineState> _drawAabbPso;

  std::shared_ptr<SamplerState> _noMipWithBorderSamplerState;

  std::shared_ptr<GpuBuffer> _aabbBuffer;
  std::shared_ptr<GpuBuffer> _shadowMapDebugBuffer;
  std::shared_ptr<VertexBuffer> _aabbVertexBuffer;
};