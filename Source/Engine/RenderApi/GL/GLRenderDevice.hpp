#pragma once
#include <array>
#include <vector>
#include <memory>
#include "GL.hpp"
#include "../RenderDevice.hpp"
#include "GLResourceSet.hpp"
#include "GLSyncPrimitives.hpp"
#include "../CommandPool.hpp"
#include "../Query.hpp"
#include "../Surface.hpp"
#include "../PresentMode.hpp"
// Presentation classes moved to separate headers for clarity
#include "GLSurface.hpp"
#include "GLSwapchain.hpp"
#include "GLQueryPool.hpp"
struct GLFWwindow;

class GLGpuBuffer;
class GLIndexBuffer;
class GLSamplerState;
class GLShaderPipeline;
class GLShaderPipelineCollection;
class GLTexture;
class GLVertexBuffer;
class ShaderParams;
class ICommandBuffer;

static const uint32 MAX_CONSTANT_BUFFERS = 32;
static const uint32 MAX_TEXTURE_SLOTS = 16;

class GLRenderDevice : public RenderDevice, public std::enable_shared_from_this<GLRenderDevice>
{
  friend class GLCommandBuffer;

public:
  GLRenderDevice(const RenderDeviceDesc &desc);

  std::shared_ptr<Shader> createShader(const ShaderDesc &desc) override;
  std::shared_ptr<VertexBuffer> createVertexBuffer(const VertexBufferDesc &desc) override;
  // Legacy RenderTarget creation removed. Use textures + Framebuffer via command buffers.
  std::shared_ptr<IndexBuffer> createIndexBuffer(const IndexBufferDesc &desc) override;
  std::shared_ptr<GpuBuffer> createGpuBuffer(const GpuBufferDesc &desc) override;
  std::shared_ptr<Texture> createTexture(const TextureDesc &desc, bool gammaCorrected = false) override;
  std::shared_ptr<SamplerState> createSamplerState(const SamplerStateDesc &desc) override;
  std::unique_ptr<IResourceSetLayout> createResourceSetLayout() override;
  std::unique_ptr<IResourceSet> createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout) override;

  /**
   * @brief Creates a command buffer for recording rendering commands
   * @return Unique pointer to the created command buffer
   * @note OpenGL implementation provides immediate mode command recording
   */
  std::unique_ptr<ICommandBuffer> createCommandBuffer() override;
  // Phase 7: queries
  std::shared_ptr<IQueryPool> createQueryPool(const QueryPoolDesc &desc);

  // Phase 8: Surfaces/Swapchain
  std::shared_ptr<ISurface> createSurface(void *nativeWindowHandle) override;
  std::shared_ptr<ISwapchain> createSwapchain(const std::shared_ptr<ISurface> &surface, const SwapchainDesc &desc) override;

  // Submission and synchronization primitives
  std::shared_ptr<IQueue> getGraphicsQueue() override;
  std::shared_ptr<IFence> createFence(bool signaled = false) override;
  std::shared_ptr<ISemaphore> createSemaphore(bool timeline = false, uint64 initialValue = 0) override;
  std::shared_ptr<ICommandPool> createCommandPool() override;

  const ViewportDesc &getViewport() const override;
  ScissorDesc getScissorDimensions() const override;

  // Debug markers & capability logging
  void beginDebugMarker(const char *label) override;
  void insertDebugMarker(const char *label) override;
  void endDebugMarker() override;
  void logCapabilities() const override;

  // Make these members accessible to GLCommandBuffer as a friend class
  // legacy pipeline state removed
  std::shared_ptr<ShaderParams> _shaderParams;
  std::array<std::shared_ptr<GLGpuBuffer>, MAX_CONSTANT_BUFFERS> _boundConstantBuffers;
  std::array<std::shared_ptr<GLTexture>, MAX_TEXTURE_SLOTS> _boundTextures;
  bool _shaderStateChanged;
  // Legacy bound render target removed.

  // Viewport and scissor state accessible to command buffer
  ScissorDesc _scissorDesc;
  ViewportDesc _viewportDesc;

  // Shader pipeline collection accessible to command buffer for pipeline management
  std::shared_ptr<GLShaderPipelineCollection> _shaderPipelineCollection;

  // State setting methods accessible to command buffer
  void setRasterizerState(const std::shared_ptr<RasterizerState> &rasterizerState);
  void setDepthStencilState(const std::shared_ptr<DepthStencilState> &depthStencilState);
  void setBlendState(const std::shared_ptr<BlendState> &blendState);

private:
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

  uint32 _stencilReadMask;
  uint32 _stencilRefValue;
  uint32 _stencilWriteMask;

  std::shared_ptr<RasterizerState> _rasterizerState;
  std::shared_ptr<DepthStencilState> _depthStencilState;
  std::shared_ptr<BlendState> _blendState;

  std::array<std::shared_ptr<GLSamplerState>, MAX_TEXTURE_SLOTS> _boundSamplers;

  std::unique_ptr<GLResourceSetFactory> _resourceSetFactory;

  // Single graphics queue for GL backend
  std::shared_ptr<GLQueue> _graphicsQueue;
};

// GL implementation of query pool
// GLQueryPool is defined in GLQueryPool.hpp