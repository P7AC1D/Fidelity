#pragma once
#include "../RenderDevice.hpp"

class GLRenderDevice : public RenderDevice
{
public:
  GLRenderDevice(const RenderDeviceDesc& desc);

  std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferDesc& desc) override;
  std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetDesc& desc) override;
  
  void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;
  void SetViewport(const ViewportDesc& viewport) override;
  void SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget) override;
  
  void Draw(uint32 vertexCount, uint32 vertexOffset) override;
  void DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) override;
  
private:
  PrimitiveTopology _primitiveTopology;
};
