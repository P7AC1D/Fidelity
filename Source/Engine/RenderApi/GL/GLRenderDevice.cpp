#include "GLRenderDevice.hpp"

#include "GLGpuBuffer.hpp"

GLRenderDevice::GLRenderDevice(const RenderDeviceDesc& desc) : RenderDevice(desc)
{
}

std::shared_ptr<GpuBuffer> GLRenderDevice::CreateGpuBuffer(const GpuBufferDesc& desc)
{
  return std::shared_ptr<GLGpuBuffer>(new GLGpuBuffer(desc));
}