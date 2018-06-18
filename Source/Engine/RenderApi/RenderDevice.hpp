#pragma once
#include "BlendState.hpp"
#include "DepthStencilState.hpp"
#include "GpuBuffer.hpp"
#include "IndexBuffer.hpp"
#include "PipelineState.hpp"
#include "RasterizerState.hpp"
#include "RenderTarget.hpp"
#include "SamplerState.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
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

struct ScissorDesc
{
	uint32 X;
	uint32 Y;
	uint32 W;
	uint32 H;
};

struct RenderDeviceDesc
{
  uint32 FrameCount = 2;
  uint32 RenderWidth;
  uint32 RenderHeight;
  bool FullscreenEnabled = false;
  bool VsyncEnabled = false;
};

enum RenderTargetType
{
	RTT_Colour = 1,
	RTT_Depth = 2,
	RTT_Stencil = 4
};

class RenderDevice
{
public:
  RenderDevice(const RenderDeviceDesc& desc): _desc(desc) {}
  
  virtual std::shared_ptr<Shader> CreateShader(const ShaderDesc& desc) = 0;
  virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(const IndexBufferDesc& desc) = 0;
  virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(const VertexBufferDesc& desc) = 0;
  virtual std::shared_ptr<Texture> CreateTexture(const TextureDesc& desc) = 0;
  virtual std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetDesc& desc) = 0;
  virtual std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferDesc& desc) = 0;
  virtual std::shared_ptr<SamplerState> CreateSamplerState(const SamplerStateDesc& desc) = 0;

  virtual void SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState) = 0;
  virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
  virtual void SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture) = 0;
  virtual void SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget) = 0;
  virtual void SetViewport(const ViewportDesc& viewport) = 0;
  virtual void SetVertexBuffer(uint32 slot, const std::shared_ptr<VertexBuffer> vertexBuffer) = 0;
  virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
  virtual void SetConstantBuffer(uint32 slot, const std::shared_ptr<GpuBuffer>& constantBuffer) = 0;
  virtual void SetSamplerState(uint32 slot, const std::shared_ptr<SamplerState>& samplerState) = 0;

	virtual void SetScissorDimensions(const ScissorDesc& desc) = 0;
	virtual ScissorDesc GetScissorDimensions() const = 0;

  virtual void Draw(uint32 vertexCount, uint32 vertexOffset) = 0;
  virtual void DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) = 0;

	virtual void ClearBuffers(uint32 buffers, const Colour& colour = Colour::Black, float32 depth = 1.0f, int32 stencil = 0) = 0;
  
  virtual std::shared_ptr<BlendState> CreateBlendState(const BlendStateDesc& desc)
  {
    return std::shared_ptr<BlendState>(new BlendState(desc));
  }
  virtual std::shared_ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& desc)
  {
    return std::shared_ptr<DepthStencilState>(new DepthStencilState(desc));
  }
  virtual std::shared_ptr<PipelineState> CreatePipelineState(const PipelineStateDesc& desc)
  {
    return std::shared_ptr<PipelineState>(new PipelineState(desc));
  }
  virtual std::shared_ptr<RasterizerState> CreateRasterizerState(const RasterizerStateDesc& desc)
  {
    return std::shared_ptr<RasterizerState>(new RasterizerState(desc));
  }
  virtual std::shared_ptr<VertexLayout> CreateVertexLayout(const std::vector<VertexLayoutDesc>& desc)
  {
    return std::shared_ptr<VertexLayout>(new VertexLayout(desc));
  }

	uint32 GetRenderWidth() const { return _desc.RenderWidth; }
	uint32 GetRenderHeight() const { return _desc.RenderHeight; }

protected:
  RenderDeviceDesc _desc;
};
