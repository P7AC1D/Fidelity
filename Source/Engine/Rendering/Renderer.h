#pragma once
#include <memory>

#include "../Core/Maths.h"
#include "../Core/Types.hpp"

class GpuBuffer;
class RenderDevice;
class SamplerState;
class VertexBuffer;

struct CameraBufferData
{
  float32 NearClip;
  float32 FarClip;
};

struct ObjectBuffer
{
  Matrix4 Model;
  Matrix4 ModelView;
  Matrix4 ModelViewProjection;
};

struct CascadeShadowMapData
{
  Matrix4 LightTransforms[4];
  // Vector4 to ensure the correct padding is done. A float will be padded by 3 x floats.
  Vector4 CascadePlaneDistances[4];
  Vector3 LightDirection;
  uint32 CascadeCount;
};

class Renderer
{
public:
  bool init(const std::shared_ptr<RenderDevice> &renderDevice);
  void drawDebugUi();

  virtual void onInit(const std::shared_ptr<RenderDevice> &renderDevice) = 0;
  virtual void onDrawDebugUi() = 0;

protected:
  Renderer();

  bool _settingsModified;

  std::shared_ptr<GpuBuffer> _cameraBuffer;

  std::shared_ptr<SamplerState> _basicSamplerState;
  std::shared_ptr<SamplerState> _noMipSamplerState;

  std::shared_ptr<VertexBuffer> _fsQuadBuffer;
};