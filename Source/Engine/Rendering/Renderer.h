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

class Renderer
{
public:
  bool init(const std::shared_ptr<RenderDevice> &renderDevice);
  void drawDebugUi();

  virtual void onInit(const std::shared_ptr<RenderDevice> &renderDevice) = 0;
  virtual void onDrawDebugUi() = 0;

protected:
  Renderer() = default;

  std::shared_ptr<GpuBuffer> _cameraBuffer;

  std::shared_ptr<SamplerState> _basicSamplerState;
  std::shared_ptr<SamplerState> _noMipSamplerState;

  std::shared_ptr<VertexBuffer> _fsQuadBuffer;
};