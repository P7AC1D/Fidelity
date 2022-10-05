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
                 const std::shared_ptr<Camera> &camera,
                 const Vector3 &sceneMaxExtents, const Vector3 &sceneMinExtents);

  std::shared_ptr<RenderTarget> getShadowMapRto() { return _shadowMapRto; }
  std::shared_ptr<GpuBuffer> getShadowBuffer() { return _cascadeShadowBuffer; }

private:
  std::vector<Matrix4> calculateCameraCascadeProjections(const std::shared_ptr<Camera> &camera);
  std::vector<float32> calculateCascadeLevels(float32 nearClip, float32 farClip);
  std::vector<Matrix4> calculateCascadeLightTransforms(const std::shared_ptr<Camera> &camera, const std::shared_ptr<Light> &directionalLight);

  void writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const;

  int32 _shadowMapResolution;
  uint32 _cascadeCount;
  bool _drawCascadeLayers;
  uint32 _sampleCount;
  float32 _sampleSpread;
  float32 _minCascadeDistance, _maxCascadeDistance;
  float32 _cascadeLambda;

  std::shared_ptr<Texture> _shadowCubeMap, _randomRotationsMap;
  std::shared_ptr<RenderTarget> _shadowMapRto;
  std::shared_ptr<PipelineState> _shadowMapPso;
  std::shared_ptr<GpuBuffer> _objectBuffer;
  std::shared_ptr<GpuBuffer> _transformsBuffer;
  std::shared_ptr<GpuBuffer> _cascadeShadowBuffer;
};