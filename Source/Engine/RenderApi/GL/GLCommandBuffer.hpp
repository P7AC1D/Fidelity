#pragma once
#include "../CommandBuffer.hpp"
#include "../GpuBuffer.hpp"
#include "../PipelineState.hpp"
#include <memory>

// Forward declarations
class GLRenderDevice;
class GLRenderTarget;
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
  void beginRenderPass(const std::shared_ptr<RenderTarget> &renderTarget,
                       bool clearColor = true,
                       bool clearDepth = true,
                       bool clearStencil = false) override;
  void endRenderPass() override;

  // Pipeline and Resource Binding
  void setPipelineState(const std::shared_ptr<PipelineState> &pipelineState) override;
  void bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex = 0) override;
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

  // Memory Barriers and Synchronization
  void memoryBarrier(uint32 srcStage, uint32 dstStage) override;

  // Render Target Management (OpenGL-specific)

  /**
   * @brief Sets the active render target for subsequent draw calls
   * @param renderTarget The render target to render into, or nullptr for main framebuffer
   * @note Command buffer must be in InRenderPass state
   * @note All rendering operations will target the specified render target
   */
  void setRenderTarget(const std::shared_ptr<RenderTarget> &renderTarget);

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
  std::shared_ptr<GLRenderDevice> _device;            ///< Parent OpenGL render device
  CommandBufferState _state;                          ///< Current command buffer state
  CommandBufferUsage _usage;                          ///< Usage hint for optimization
  std::shared_ptr<RenderTarget> _currentRenderTarget; ///< Currently active render target

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
};
