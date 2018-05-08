#pragma once
#include "BlendState.hpp"
#include "DepthStencilState.hpp"
#include "GpuBuffer.hpp"
#include "PipelineState.hpp"
#include "RasterizerState.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexLayout.hpp"

struct ViewportDesc
{
  float32 TopLeftX;
  float32 TopLeftY;
  float32 Width;
  float32 Height;
  float32 MinDepth;
  float32 MaxDepth;
};

struct RenderDeviceDesc
{
  uint32 FrameCount = 2;
  uint32 RenderWidth;
  uint32 RenderHeight;
  bool FullscreenEnabled = false;
  bool VsyncEnabled = false;
};

class RenderDevice
{
public:
  RenderDevice(const RenderDeviceDesc& desc): _desc(desc) {}

  virtual std::shared_ptr<BlendState> CreateBlendState(const BlendStateDesc& desc) = 0;
  virtual std::shared_ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& desc) = 0;
  virtual std::shared_ptr<PipelineState> CreatePipelineState(const PipelineStateDesc& desc) = 0;
  virtual std::shared_ptr<RasterizerState> CreateRasterizerState(const RasterizerStateDesc& desc) = 0;
  virtual std::shared_ptr<VertexLayout> CreateVertexLayout(const VertexLayoutDesc& desc) = 0;
  virtual std::shared_ptr<Shader> CreateShader(const ShaderDesc& desc) = 0;
  virtual std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferDesc& desc) = 0;
  virtual std::shared_ptr<Texture> CreateTexture(const TextureDesc& desc) = 0;
  virtual std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetDesc& desc) = 0;

  virtual void SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState) = 0;
  virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
  virtual void SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture) = 0;
  virtual void SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget) = 0;
  virtual void SetViewport(const ViewportDesc& viewport) = 0;

  virtual void Draw(uint32 vertexCount, uint32 vertexOffset) = 0;
  virtual void DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) = 0;

protected:
  RenderDeviceDesc _desc;
};
