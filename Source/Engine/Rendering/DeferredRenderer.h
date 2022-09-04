#pragma once
#include <memory>
#include <vector>

#include "../Core/Maths.h"

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

enum class DebugDisplayType
{
  Disabled,
  Diffuse,
  Normal,
  Depth,
  Emissive,
  Specular
};

class DeferredRenderer
{
public:
  DeferredRenderer(const Vector2I &windowDims);

  bool init(std::shared_ptr<RenderDevice> renderDevice);

  void drawFrame(std::shared_ptr<RenderDevice> renderDevice,
                 const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                 const std::vector<std::shared_ptr<Drawable>> &drawables,
                 const std::vector<std::shared_ptr<Light>> &lights,
                 const Camera &camera);

  void setDebugDisplayType(DebugDisplayType debugDisplayType) { _debugDisplayType = debugDisplayType; }

private:
  void gbufferPass(std::shared_ptr<RenderDevice> renderDevice,
                   const std::vector<std::shared_ptr<Drawable>> &drawables,
                   const Camera &camera);

  void lightPrePass(std::shared_ptr<RenderDevice> renderDevice,
                    const std::vector<std::shared_ptr<Light>> &lights,
                    const Camera &camera);

  void mergePass(std::shared_ptr<RenderDevice> renderDevice);

  void drawRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                        std::shared_ptr<Texture> renderTarget,
                        const Camera &camera);

  void drawAabb(std::shared_ptr<RenderDevice> renderDevice,
                const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                const Camera &camera);

  void writeMaterialConstantData(std::shared_ptr<RenderDevice> renderDevice,
                                 std::shared_ptr<Material> material) const;
  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const Camera &camera) const;

  Vector2I _windowDims;
  DebugDisplayType _debugDisplayType;

  std::shared_ptr<PipelineState> _mergePso;
  std::shared_ptr<PipelineState> _lightPrePassCWPto, _lightPrePassCCWPto;
  std::shared_ptr<PipelineState> _gBufferPso;
  std::shared_ptr<PipelineState> _gbufferDebugDrawPso;
  std::shared_ptr<PipelineState> _depthDebugDrawPso;
  std::shared_ptr<PipelineState> _drawAabbPso;
  std::shared_ptr<RenderTarget> _lightPrePassRto;
  std::shared_ptr<SamplerState> _basicSamplerState;
  std::shared_ptr<SamplerState> _noMipSamplerState;
  std::shared_ptr<RenderTarget> _gBufferRto;
  std::shared_ptr<GpuBuffer> _materialBuffer;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _pointLightConstantsBuffer;
  std::shared_ptr<GpuBuffer> _pointLightBuffer;
  std::shared_ptr<GpuBuffer> _cameraBuffer;
  std::shared_ptr<GpuBuffer> _aabbBuffer;
  std::shared_ptr<StaticMesh> _pointLightMesh;
  std::shared_ptr<VertexBuffer> _fsQuadBuffer;
  std::shared_ptr<VertexBuffer> _aabbVertexBuffer;
};