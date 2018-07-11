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
class GLVertexArrayObject;
class GLVertexArrayObjectCollection;
class GLVertexBuffer;
class ShaderParams;

static const uint32 MaxConstantBuffers = 32;
static const uint32 MaxTextureSlots = 16;
static const uint32 MaxVertexBuffers = 16;

class GLRenderDevice : public RenderDevice
{
public:
  GLRenderDevice(const RenderDeviceDesc& desc);

  std::shared_ptr<Shader> CreateShader(const ShaderDesc& desc) override;
  std::shared_ptr<VertexBuffer> CreateVertexBuffer(const VertexBufferDesc& desc) override;
  std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetDesc& desc) override;
  std::shared_ptr<IndexBuffer> CreateIndexBuffer(const IndexBufferDesc& desc) override;
  std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferDesc& desc) override;
  std::shared_ptr<Texture> CreateTexture(const TextureDesc& desc) override;
  std::shared_ptr<SamplerState> CreateSamplerState(const SamplerStateDesc& desc) override;
  
  void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;
  void SetViewport(const ViewportDesc& viewport) override;
  void SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState) override;
  void SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget) override;
  void SetVertexBuffer(uint32 slot, const std::shared_ptr<VertexBuffer> vertexBuffer) override;
  void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
  void SetConstantBuffer(uint32 slot, const std::shared_ptr<GpuBuffer>& constantBuffer) override;
  void SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture) override;
  void SetSamplerState(uint32 slot, const std::shared_ptr<SamplerState>& samplerState) override;
	void SetScissorDimensions(const ScissorDesc& desc) override;

	const ViewportDesc& GetViewport() const override;
	ScissorDesc GetScissorDimensions() const override;
  
  void Draw(uint32 vertexCount, uint32 vertexOffset) override;
  void DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) override;

	void ClearBuffers(uint32 buffers, const Colour& colour = Colour(115, 140, 153, 255), float32 depth = 1.0f, int32 stencil = 0) override;
  
private:
  void BeginDraw();
  void EndDraw();

  void SetRasterizerState(const std::shared_ptr<RasterizerState>& rasterizerState);
  void SetDepthStencilState(const std::shared_ptr<DepthStencilState>& depthStencilState);
  void SetBlendState(const std::shared_ptr<BlendState>& blendState);
  
  void SetDepthBias(float32 constantBias, float32 slopeScaleBias);
  void SetCullingMode(CullMode cullMode);
  void SetFillMode(FillMode fillMode);
  void SetStencilOperations(const StencilOperationDesc& stencilOperationDesc, bool isFrontFace);
  void SetStencilFunction(ComparisonFunction comparisonFunc, uint32 readMask, bool isFrontFace);
  void SetStencilWriteMask(uint32 writeMask);
  void SetDepthFunction(ComparisonFunction depthFunc);
  void SetBlendFactors(BlendFactor srcFactor, BlendFactor dstFactor, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor);
  void SetBlendOperation(BlendOperation op, BlendOperation alphaOp);
  void SetBlendWriteMask(byte writeMask);
  
  void EnableScissorTest(bool enableScissorTest);
  void EnableMultisampling(bool enableMultisampling);
  void EnableDepthClip(bool enableDepthClip);
  void EnableAntialiasedLine(bool enableAntialiasedLine);
  void EnableStencilTest(bool enableStencilTest);
  void EnableDepthTest(bool enableDepthTest);
  void EnableDepthWrite(bool enableDepthWrite);
  void EnableBlend(bool enableBlend);
  
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
  std::shared_ptr<RasterizerState> _rasterizerState;
  std::shared_ptr<DepthStencilState> _depthStencilState;
  std::shared_ptr<BlendState> _blendState;
  std::shared_ptr<PipelineState> _pipelineState;
  std::shared_ptr<ShaderParams> _shaderParams;
  
  std::array<std::shared_ptr<GLVertexBuffer>, MaxVertexBuffers> _boundVertexBuffers;
  std::array<std::shared_ptr<GLGpuBuffer>, MaxConstantBuffers> _boundConstantBuffers;
  std::array<std::shared_ptr<GLTexture>, MaxTextureSlots> _boundTextures;
  std::array<std::shared_ptr<GLSamplerState>, MaxTextureSlots> _boundSamplers;
  
  std::shared_ptr<GLShaderPipelineCollection> _shaderPipelineCollection;
  std::shared_ptr<GLVertexArrayObjectCollection> _vaoCollection;
};
