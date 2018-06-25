#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"

class Camera;
class GpuBuffer;
class PipelineState;
class SamplerState;
class RenderDevice;
class RenderTarget;
class Renderable;
class Transform;
class VertexBuffer;

struct DirectionalLightData
{
	DirectionalLightData():
		Colour(Colour::White),
		Direction(-Vector3::Identity),
		Intensity(1.0f)
	{}

	DirectionalLightData(const Colour& colour, const Vector3& direction, float32 intensity):
		Colour(colour),
		Direction(direction),
		Intensity(intensity)
	{}

	Colour Colour;
	Vector3 Direction;
	float32 Intensity;
};

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
  GL41
};

enum class GBufferDisplayType
{
	Disabled,
	Position,
	Normal,
	Albedo
};
  
struct RendererDesc
{
  uint32 RenderWidth;
  uint32 RenderHeight;
  RenderApi RenderApi = RenderApi::GL41;
  bool FullscreenEnabled = false;
  bool VsyncEnabled = false;
};

class Renderer
{
public:
	static std::shared_ptr<RenderDevice> GetRenderDevice();

  Renderer(const RendererDesc& desc);
  
  uint32 GetRenderWidth() const { return _desc.RenderWidth; }
  uint32 GetRenderHeight() const { return _desc.RenderHeight; }
  
  void SetCamera(const std::shared_ptr<Camera>& camera) { _activeCamera = camera; }
	void SetDirectionalLight(const DirectionalLightData& lightData) { _directionalLight = lightData; }

	void EnableGBufferDebugPass(GBufferDisplayType gBufferDisplay) { _gBufferDisplay = gBufferDisplay; }
  
  void PushRenderable(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform);
  
  void DrawFrame();
  
private:
  void InitPipelineStates();
  void InitCameraBuffer();
	void InitObjectBuffer();
	void InitLightingPass();
	void InitFullscreenQuad();
	void InitGBufferDebugPass();

  void StartFrame();
	void EndFrame();

	void GeometryPass();
	void LightingPass();
	void GBufferDebugPass(uint32 i);

private:
  static std::shared_ptr<RenderDevice> _renderDevice;
  RendererDesc _desc;
	GBufferDisplayType _gBufferDisplay;
	DirectionalLightData _directionalLight;

  std::shared_ptr<GpuBuffer> _frameBuffer;
	std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<PipelineState> _geomPassPso;
	std::shared_ptr<PipelineState> _lightPassPso;
	std::shared_ptr<PipelineState> _gBufferDebugPso;
  std::shared_ptr<Camera> _activeCamera;
	std::shared_ptr<SamplerState> _basicSamplerState;
	std::shared_ptr<SamplerState> _noMipSamplerState;
	std::shared_ptr<RenderTarget> _gBuffer;
	std::shared_ptr<VertexBuffer> _fsQuadBuffer;
  
  std::vector<RenderableItem> _renderables;
};
