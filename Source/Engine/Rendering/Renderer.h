#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Core/Types.hpp"

class Camera;
class GpuBuffer;
class PipelineState;
class RenderDevice;
class Renderable;
class Transform;

struct RenderableItem
{
  RenderableItem(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform):
    Renderable(renderable),
    Transform(transform)
  {}
  
  std::shared_ptr<Renderable> Renderable;
  std::shared_ptr<Transform> Transform;
};

enum class RenderApi
{
  GL40
};
  
struct RendererDesc
{
  uint32 RenderWidth;
  uint32 RenderHeight;
  RenderApi RenderApi = RenderApi::GL40;
  bool FullscreenEnabled = false;
  bool VsyncEnabled = false;
};

class Renderer
{
public:
  Renderer(const RendererDesc& desc);
  
  uint32 GetRenderWidth() const { return _desc.RenderWidth; }
  uint32 GetRenderHeight() const { return _desc.RenderHeight; }
  
  void SetCamera(const std::shared_ptr<Camera>& camera) { _activeCamera = camera; }
  
  void PushRenderable(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform);
  
  void DrawFrame();
  
private:
  void InitPipelineStates();
  void InitConstBuffer();
  void StartFrame();

private:
  RendererDesc _desc;
  std::shared_ptr<RenderDevice> _renderDevice;
  std::shared_ptr<GpuBuffer> _constBuffer;
  std::shared_ptr<PipelineState> _basicPipeline;
  std::shared_ptr<Camera> _activeCamera;
  
  std::vector<RenderableItem> _renderables;
};
