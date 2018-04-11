#pragma once
#include "../RenderDevice.hpp"

class GLRenderDevice : public RenderDevice
{
public:
  GLRenderDevice(const RenderDeviceDesc& desc);

  std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferDesc& desc) override;
};