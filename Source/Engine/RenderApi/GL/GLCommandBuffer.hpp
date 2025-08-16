#pragma once
#include "../CommandBuffer.hpp"
#include "../GpuBuffer.hpp"
#include "../PrimitiveTopology.hpp"
#include "../PipelineLayout.hpp"
#include "../PushConstants.hpp"
#include "../GraphicsPipelineState.hpp"
#include "../ComputePipelineState.hpp"
#include <memory>
#include <vector>

// Forward declarations
class GLRenderDevice;
class GLPipelineState;
class GLVertexBuffer;
class GLIndexBuffer;
class GLTexture;
class GLGpuBuffer;
class GLShaderPipeline;
class GLShaderPipelineCollection;
class IResourceSet;
class Colour;

/**
 * @brief OpenGL implementation of the command buffer interface
 *
 * GLCommandBuffer provides a command buffer abstraction over OpenGL's immediate
 * mode rendering. While OpenGL doesn't have native command buffer support,
 * this implementation records commands and executes them immediately to maintain
 * compatibility with the abstract command buffer interface.
 *
 * This implementation serves as a bridge to allow gradual migration to modern
 * graphics APIs while maintaining the existing OpenGL backend functionality.
 *
 * @note OpenGL commands are executed immediately upon recording
 * @note State validation ensures proper usage patterns are maintained
 */
class GLCommandBuffer : public ICommandBuffer
{
public:
  /**
   * @brief Constructs an OpenGL command buffer
   * @param device Shared pointer to the parent OpenGL render device
   */
  explicit GLCommandBuffer(std::shared_ptr<GLRenderDevice> device);

  virtual ~GLCommandBuffer() = default;

  // Command Buffer Lifecycle
  void begin(CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) override;
  void end() override;
  void execute() override;
  void reset() override;

  // Render Pass Management
  // Legacy beginRenderPass(RenderTarget,...) removed; use RenderPassBeginInfo overload
  void beginRenderPass(const RenderPassBeginInfo &beginInfo) override;
  void nextSubpass() override;
  void endRenderPass() override;

  // Pipeline and Resource Binding
  // legacy setPipelineState removed
  void bindGraphicsPipeline(const std::shared_ptr<class GraphicsPipelineState> &pipeline) override;
  void bindComputePipeline(const std::shared_ptr<class ComputePipelineState> &pipeline) override;
  void bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex = 0) override;
  void bindDescriptorSets(PipelineBindPoint bindPoint,
                          const std::shared_ptr<PipelineLayout> &layout,
                          uint32 firstSet,
                          const std::vector<const IResourceSet *> &sets,
                          const uint32 *dynamicOffsets = nullptr,
                          uint32 dynamicOffsetCount = 0) override;
  void pushConstants(const std::shared_ptr<PipelineLayout> &layout,
                     uint32 stageMask,
                     uint32 offset,
                     uint32 size,
                     const void *data) override;
  void bindVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer,
                        uint32 bindingIndex = 0,
                        uint64 offset = 0) override;
  void bindIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer, uint64 offset = 0) override;

  // Dynamic State
  void setViewport(const ViewportDesc &viewport) override;
  void setScissor(const ScissorDesc &scissor) override;

  /**
   * @brief Sets the primitive topology for subsequent draw calls
   * @param topology The primitive topology to use (triangles, lines, etc.)
   * @note Command buffer must be in InRenderPass state
   * @note This overrides the topology from the pipeline state for dynamic topology changes
   */
  void setPrimitiveTopology(PrimitiveTopology topology) override;

  // Drawing Commands
  void draw(uint32 vertexCount,
            uint32 instanceCount = 1,
            uint32 firstVertex = 0,
            uint32 firstInstance = 0) override;
  void drawIndexed(uint32 indexCount,
                   uint32 instanceCount = 1,
                   uint32 firstIndex = 0,
                   int32 vertexOffset = 0,
                   uint32 firstInstance = 0) override;

  // Indirect
  void drawIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset) override;
  void drawIndexedIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset) override;

  // Compute
  void dispatch(uint32 x, uint32 y, uint32 z) override;
  void dispatchIndirect(const std::shared_ptr<GpuBuffer> &buffer, uint64 offset) override;

  // Resource Operations
  void copyBuffer(const std::shared_ptr<GpuBuffer> &srcBuffer,
                  const std::shared_ptr<GpuBuffer> &dstBuffer,
                  uint64 srcOffset,
                  uint64 dstOffset,
                  uint64 size) override;
  void copyBufferToTexture(const std::shared_ptr<GpuBuffer> &srcBuffer,
                           const std::shared_ptr<Texture> &dstTexture,
                           uint64 bufferOffset = 0,
                           uint32 mipLevel = 0,
                           uint32 arrayLayer = 0) override;
  void updateBuffer(const std::shared_ptr<GpuBuffer> &buffer,
                    uint64 offset,
                    uint64 size,
                    const void *data,
                    AccessType accessType = AccessType::WriteOnly) override;

  // Texture ops (Phase 7)
  void copyTexture(const std::shared_ptr<Texture> &src,
                   const std::shared_ptr<Texture> &dst,
                   uint32 srcMipLevel = 0,
                   uint32 dstMipLevel = 0) override;
  void blitTexture(const std::shared_ptr<Texture> &src,
                   const std::shared_ptr<Texture> &dst,
                   bool linearFilter = false,
                   uint32 srcMipLevel = 0,
                   uint32 dstMipLevel = 0) override;
  void resolveTexture(const std::shared_ptr<Texture> &srcMsaa,
                      const std::shared_ptr<Texture> &dstSingle,
                      uint32 srcMipLevel = 0,
                      uint32 dstMipLevel = 0) override;
  void generateMips(const std::shared_ptr<Texture> &texture) override;

  // Depth-only blit helper
  void blitDepthToDefault(const std::shared_ptr<Texture> &srcDepth) override;

  // Memory Barriers and Synchronization
  void memoryBarrier(uint32 srcStage, uint32 dstStage) override;
  void pipelineBarrier(uint32 srcStages,
                       uint32 dstStages,
                       uint32 memoryDeps,
                       const std::vector<BufferBarrier> &bufferBarriers,
                       const std::vector<ImageBarrier> &imageBarriers) override;
  void transition(const std::shared_ptr<ImageView> &view,
                  ResourceState oldState,
                  ResourceState newState,
                  ImageAspect aspect = ImageAspect::Color) override;

  // Queries & timestamps
  void writeTimestamp(const std::shared_ptr<IQueryPool> &pool, uint32 index) override;
  void beginQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index) override;
  void endQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index) override;
  void resolveQueryData(const std::shared_ptr<IQueryPool> &pool, uint32 first, uint32 count, uint64 *dst) override;

  // Render Target Management (OpenGL-specific)
  /**
   * @brief Clears the currently bound render target buffers
   * @param buffers Bitwise combination of RenderTargetType flags specifying which buffers to clear
   * @param colour Clear color for color buffers
   * @param depth Clear value for depth buffer
   * @param stencil Clear value for stencil buffer
   * @note Command buffer must be in InRenderPass state
   */
  void clearRenderTarget(uint32 buffers, const Colour &colour, float32 depth = 1.0f, int32 stencil = 0);

  /**
   * @brief Clears the currently bound render target buffers with default black color
   * @param buffers Bitwise combination of RenderTargetType flags specifying which buffers to clear
   * @param depth Clear value for depth buffer
   * @param stencil Clear value for stencil buffer
   * @note Command buffer must be in InRenderPass state
   */
  void clearRenderTarget(uint32 buffers, float32 depth = 1.0f, int32 stencil = 0);

  // State Queries
  CommandBufferState getState() const override;
  bool isRecording() const override;
  bool isInRenderPass() const override;

private:
  /**
   * @brief Prepares OpenGL state for drawing operations
   * @note Validates pipeline state and sets up vertex array objects
   */
  void beginDraw();

  /**
   * @brief Cleans up OpenGL state after drawing operations
   * @note Unbinds vertex array objects
   */
  void endDraw();
  /**
   * @brief Validates command buffer state for the given operation
   * @param expectedState The expected state for the operation
   * @param operationName Name of the operation for error reporting
   */
  void validateState(CommandBufferState expectedState, const char *operationName) const;

  /**
   * @brief Validates that command buffer is in render pass for draw operations
   * @param operationName Name of the operation for error reporting
   */
  void validateInRenderPass(const char *operationName) const;

  /**
   * @brief Validates that command buffer is not in render pass for resource operations
   * @param operationName Name of the operation for error reporting
   */
  void validateOutsideRenderPass(const char *operationName) const;

private:
  std::shared_ptr<GLRenderDevice> _device; ///< Parent OpenGL render device
  CommandBufferState _state;               ///< Current command buffer state
  CommandBufferUsage _usage;               ///< Usage hint for optimization
  uint32 _currentFboId = 0;                ///< Currently bound FBO (0 = default)
  bool _ownsCurrentFbo = false;            ///< Whether the command buffer created the FBO

  // Command buffer owns its rendering state (following modern graphics API patterns)
  PrimitiveTopology _primitiveTopology;                                  ///< Current primitive topology for draw calls
  std::shared_ptr<GLIndexBuffer> _boundIndexBuffer;                      ///< Currently bound index buffer
  std::shared_ptr<GLVertexBuffer> _boundVertexBuffer;                    ///< Currently bound vertex buffer
  std::shared_ptr<GLShaderPipeline> _shaderPipeline;                     ///< Active shader pipeline for rendering
  std::shared_ptr<GLShaderPipelineCollection> _shaderPipelineCollection; ///< Collection for shader pipeline management

  // Cached state for validation and optimization
  bool _hasActivePipeline;     ///< Whether a pipeline state is bound
  bool _hasActiveVertexBuffer; ///< Whether a vertex buffer is bound
  bool _hasActiveIndexBuffer;  ///< Whether an index buffer is bound

  // New pipeline state (migration)
  std::shared_ptr<GraphicsPipelineState> _boundGraphicsPipeline;
  std::shared_ptr<ComputePipelineState> _boundComputePipeline; // unused on GL 4.1
  bool _warnedComputeUnsupported = false;                      // throttle warnings
};
