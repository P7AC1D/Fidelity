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
                 const Camera &camera);

private:
  std::vector<Vector4> getFrustrumCorners(const Matrix4 &proj, const Matrix4 &cameraView) const;
  Matrix4 calcLightViewProj(const float32 nearPlane,
                            const float32 farPlane,
                            const Camera &camera,
                            const std::shared_ptr<Light> &directionalLight) const;
  Matrix4 calcLightView(const std::vector<Vector4> &frustrumCorners, const std::shared_ptr<Light> &directionalLight) const;
  Matrix4 calcLightProj(const std::vector<Vector4> &frustrumCorners, const Matrix4 &lightView) const;

  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const Camera &camera) const;
  void writeTransformConstantData(const std::shared_ptr<RenderDevice> &renderDevice,
                                  const std::shared_ptr<Light> &directionalLight,
                                  const Camera &camera) const;

  float32 _zMulti;
  uint32 _shadowMapResolution;
  uint32 _cascadeCount;

  std::shared_ptr<Texture> _shadowCubeMap;
  std::shared_ptr<RenderTarget> _shadowMapRto;
  std::shared_ptr<SamplerState> _shadowMapSamplerState;
  std::shared_ptr<PipelineState> _shadowMapPso;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _transformsBuffer;
};