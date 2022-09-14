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
  std::shared_ptr<GpuBuffer> getCSMBuffer() { return _cascadeShadowBuffer; }

private:
  std::vector<Vector4>
  getFrustrumCorners(const Matrix4 &proj, const Matrix4 &cameraView) const;
  Matrix4 calcLightViewProj(const float32 nearPlane,
                            const float32 farPlane,
                            const std::shared_ptr<Camera> &camera,
                            const std::shared_ptr<Light> &directionalLight) const;
  Matrix4 calcLightView(const std::vector<Vector4> &frustrumCorners, const std::shared_ptr<Light> &directionalLight) const;
  Matrix4 calcLightProj(const std::vector<Vector4> &frustrumCorners, const Matrix4 &lightView) const;

  std::vector<Matrix4> createLightTransforms(const std::shared_ptr<Light> &directionalLight,
                                             const std::shared_ptr<Camera> &camera) const;
  std::vector<float32> createCascadeLevels(const std::shared_ptr<Camera> &camera) const;

  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const;

  float32 _zMulti;
  int32 _shadowMapResolution;
  uint32 _cascadeCount;

  std::shared_ptr<Texture> _shadowCubeMap;
  std::shared_ptr<RenderTarget> _shadowMapRto;
  std::shared_ptr<SamplerState> _shadowMapSamplerState;
  std::shared_ptr<PipelineState> _shadowMapPso;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _transformsBuffer;
  std::shared_ptr<GpuBuffer> _cascadeShadowBuffer;
};