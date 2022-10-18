#pragma once
#include <array>
#include "../RenderDevice.hpp"

class GLGpuBuffer;
class GLIndexBuffer;
class GLRenderTarget;
class GLSamplerState;
class GLShaderPipeline;
class GLShaderPipelineCollection;
class GLTexture;
class GLVertexBuffer;
class ShaderParams;

static const uint32 MAX_CONSTANT_BUFFERS = 32;
static const uint32 MAX_TEXTURE_SLOTS = 16;

class GLRenderDevice : public RenderDevice
{
public:
  GLRenderDevice(const RenderDeviceDesc &desc);

  std::shared_ptr<Shader> createShader(const ShaderDesc &desc) override;
  std::shared_ptr<VertexBuffer> createVertexBuffer(const VertexBufferDesc &desc) override;
  std::shared_ptr<RenderTarget> createRenderTarget(const RenderTargetDesc &desc) override;
  std::shared_ptr<IndexBuffer> createIndexBuffer(const IndexBufferDesc &desc) override;
  std::shared_ptr<GpuBuffer> createGpuBuffer(const GpuBufferDesc &desc) override;
  std::shared_ptr<Texture> createTexture(const TextureDesc &desc, bool gammaCorrected = false) override;
  std::shared_ptr<SamplerState> createSamplerState(const SamplerStateDesc &desc) override;

  void setPrimitiveTopology(PrimitiveTopology primitiveTopology) override;
  void setViewport(const ViewportDesc &viewport) override;
  void setPipelineState(const std::shared_ptr<PipelineState> &pipelineState) override;
  void setRenderTarget(const std::shared_ptr<RenderTarget> &renderTarget) override;
  void setVertexBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer) override;
  void setIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;
  void setConstantBuffer(uint32 slot, const std::shared_ptr<GpuBuffer> &constantBuffer) override;
  void setTexture(uint32 slot, const std::shared_ptr<Texture> &texture) override;
  void setSamplerState(uint32 slot, const std::shared_ptr<SamplerState> &samplerState) override;
  void setScissorDimensions(const ScissorDesc &desc) override;

  const ViewportDesc &getViewport() const override;
  ScissorDesc getScissorDimensions() const override;

  void draw(uint32 vertexCount, uint32 vertexOffset) override;
  void drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) override;

  void clearBuffers(uint32 buffers, const Colour &colour = Colour(115, 140, 153, 255), float32 depth = 1.0f, int32 stencil = 0) override;

private:
  void beginDraw();
  void endDraw();

  void setRasterizerState(const std::shared_ptr<RasterizerState> &rasterizerState);
  void setDepthStencilState(const std::shared_ptr<DepthStencilState> &depthStencilState);
  void setBlendState(const std::shared_ptr<BlendState> &blendState);

  void setDepthBias(float32 constantBias, float32 slopeScaleBias);
  void setCullingMode(CullMode cullMode);
  void setFillMode(FillMode fillMode);
  void setStencilOperations(const StencilOperationDesc &stencilOperationDesc, bool isFrontFace);
  void setStencilFunction(ComparisonFunction comparisonFunc, uint32 readMask, bool isFrontFace);
  void setStencilWriteMask(uint32 writeMask);
  void setDepthFunction(ComparisonFunction depthFunc);
  void setBlendFactors(BlendFactor srcFactor, BlendFactor dstFactor, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor);
  void setBlendOperation(BlendOperation op, BlendOperation alphaOp);
  void setBlendWriteMask(byte writeMask);

  void enableScissorTest(bool enableScissorTest);
  void enableMultisampling(bool enableMultisampling);
  void enableDepthClip(bool enableDepthClip);
  void enableAntialiasedLine(bool enableAntialiasedLine);
  void enableStencilTest(bool enableStencilTest);
  void enableDepthTest(bool enableDepthTest);
  void enableDepthWrite(bool enableDepthWrite);
  void enableBlend(bool enableBlend);

private:
  bool _shaderStateChanged;

  PrimitiveTopology _primitiveTopology;

  uint32 _stencilReadMask;
  uint32 _stencilRefValue;
  uint32 _stencilWriteMask;

  ScissorDesc _scissorDesc;
  ViewportDesc _viewportDesc;

  std::shared_ptr<GLIndexBuffer> _boundIndexBuffer;
  std::shared_ptr<GLRenderTarget> _boundRenderTarget;
  std::shared_ptr<GLShaderPipeline> _shaderPipeline;
  std::shared_ptr<GLVertexBuffer> _boundVertexBuffer;
  std::shared_ptr<RasterizerState> _rasterizerState;
  std::shared_ptr<DepthStencilState> _depthStencilState;
  std::shared_ptr<BlendState> _blendState;
  std::shared_ptr<PipelineState> _pipelineState;
  std::shared_ptr<ShaderParams> _shaderParams;

  std::array<std::shared_ptr<GLGpuBuffer>, MAX_CONSTANT_BUFFERS> _boundConstantBuffers;
  std::array<std::shared_ptr<GLTexture>, MAX_TEXTURE_SLOTS> _boundTextures;
  std::array<std::shared_ptr<GLSamplerState>, MAX_TEXTURE_SLOTS> _boundSamplers;

  std::shared_ptr<GLShaderPipelineCollection> _shaderPipelineCollection;
};
