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
  float32 TopLeftX = 0.0;
  float32 TopLeftY = 0.0;
  float32 Width;
  float32 Height;
  float32 MinDepth = 0.0f;
  float32 MaxDepth = 1.0f;
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
  RenderDevice(const RenderDeviceDesc &desc) : _desc(desc) {}

  virtual std::shared_ptr<Shader> createShader(const ShaderDesc &desc) = 0;
  virtual std::shared_ptr<IndexBuffer> createIndexBuffer(const IndexBufferDesc &desc) = 0;
  virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const VertexBufferDesc &desc) = 0;
  virtual std::shared_ptr<Texture> createTexture(const TextureDesc &desc, bool gammaCorrected = false) = 0;
  virtual std::shared_ptr<RenderTarget> createRenderTarget(const RenderTargetDesc &desc) = 0;
  virtual std::shared_ptr<GpuBuffer> createGpuBuffer(const GpuBufferDesc &desc) = 0;
  virtual std::shared_ptr<SamplerState> createSamplerState(const SamplerStateDesc &desc) = 0;

  virtual void setPipelineState(const std::shared_ptr<PipelineState> &pipelineState) = 0;
  virtual void setPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
  virtual void setTexture(uint32 slot, const std::shared_ptr<Texture> &texture) = 0;
  virtual void setRenderTarget(const std::shared_ptr<RenderTarget> &renderTarget) = 0;
  virtual void setViewport(const ViewportDesc &viewport) = 0;
  virtual void setVertexBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer) = 0;
  virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) = 0;
  virtual void setConstantBuffer(uint32 slot, const std::shared_ptr<GpuBuffer> &constantBuffer) = 0;
  virtual void setSamplerState(uint32 slot, const std::shared_ptr<SamplerState> &samplerState) = 0;
  virtual void setScissorDimensions(const ScissorDesc &desc) = 0;

  virtual const ViewportDesc &getViewport() const = 0;
  virtual ScissorDesc getScissorDimensions() const = 0;

  virtual void draw(uint32 vertexCount, uint32 vertexOffset) = 0;
  virtual void drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) = 0;

  virtual void clearBuffers(uint32 buffers, const Colour &colour = Colour::Black, float32 depth = 1.0f, int32 stencil = 0) = 0;

  virtual std::shared_ptr<BlendState> createBlendState(const BlendStateDesc &desc)
  {
    return std::shared_ptr<BlendState>(new BlendState(desc));
  }
  virtual std::shared_ptr<DepthStencilState> createDepthStencilState(const DepthStencilStateDesc &desc)
  {
    return std::shared_ptr<DepthStencilState>(new DepthStencilState(desc));
  }
  virtual std::shared_ptr<PipelineState> createPipelineState(const PipelineStateDesc &desc)
  {
    return std::shared_ptr<PipelineState>(new PipelineState(desc));
  }
  virtual std::shared_ptr<RasterizerState> createRasterizerState(const RasterizerStateDesc &desc)
  {
    return std::shared_ptr<RasterizerState>(new RasterizerState(desc));
  }
  virtual std::shared_ptr<VertexLayout> createVertexLayout(const std::vector<VertexLayoutDesc> &desc)
  {
    return std::shared_ptr<VertexLayout>(new VertexLayout(desc));
  }

  uint32 getRenderWidth() const { return _desc.RenderWidth; }
  uint32 getRenderHeight() const { return _desc.RenderHeight; }

protected:
  RenderDeviceDesc _desc;
};
