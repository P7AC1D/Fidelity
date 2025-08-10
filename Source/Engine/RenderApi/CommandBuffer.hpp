#pragma once
#include <memory>
#include "../Core/Types.hpp"
#include "GpuBuffer.hpp"

// Forward declarations
class RenderTarget;
class PipelineState;
class IResourceSet;
class VertexBuffer;
class IndexBuffer;
struct ViewportDesc;
struct ScissorDesc;
class Texture;
class GpuBuffer;
enum class PrimitiveTopology;

/**
 * @brief Command recording state for command buffers
 *
 * Tracks the current state of command recording to ensure proper
 * command buffer usage patterns across different graphics APIs.
 */
enum class CommandBufferState
{
  Initial,      ///< Command buffer is ready to begin recording
  Recording,    ///< Command buffer is actively recording commands
  InRenderPass, ///< Command buffer is recording within a render pass
  Executable    ///< Command buffer has finished recording and can be submitted
};

/**
 * @brief Command buffer usage patterns for optimization hints
 *
 * Provides hints to the graphics API about how the command buffer
 * will be used, enabling driver optimizations.
 */
enum class CommandBufferUsage
{
  OneTimeSubmit,      ///< Command buffer will be submitted once then reset
  RenderPassContinue, ///< Command buffer continues a render pass from another buffer
  SimultaneousUse     ///< Command buffer can be submitted multiple times concurrently
};

/**
 * @brief Abstract interface for command buffer implementations
 *
 * Command buffers provide a deferred command recording mechanism that is
 * essential for modern graphics APIs (Vulkan, DirectX 12, Metal). Commands
 * are recorded into the buffer and then submitted to a command queue for
 * execution on the GPU.
 *
 * The command buffer follows a strict state machine:
 * 1. Initial state - ready to begin recording
 * 2. Recording state - actively recording commands
 * 3. In render pass state - recording within a render pass
 * 4. Executable state - ready for submission
 *
 * Command buffers must be properly begun and ended, and render passes
 * must be explicitly started and stopped. This design maps efficiently
 * to Vulkan command buffers while providing fallback support for OpenGL.
 */
class ICommandBuffer
{
public:
  virtual ~ICommandBuffer() = default;

  // Command Buffer Lifecycle

  /**
   * @brief Begins command recording
   * @param usage Optional usage hint for driver optimization
   * @note Command buffer must be in Initial state
   * @note Transitions to Recording state
   */
  virtual void begin(CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) = 0;

  /**
   * @brief Ends command recording
   * @note Command buffer must be in Recording state (not in render pass)
   * @note Transitions to Executable state
   */
  virtual void end() = 0;

  /**
   * @brief Submits the command buffer for execution on the GPU
   * @note Command buffer must be in Executable state
   * @note Execution is asynchronous - use synchronization for completion detection
   * @note OpenGL implementation executes immediately, other APIs queue for execution
   */
  virtual void execute() = 0;

  /**
   * @brief Resets command buffer to initial state
   * @note Command buffer must not be pending execution on GPU
   * @note Transitions to Initial state
   */
  virtual void reset() = 0;

  // Render Pass Management

  /**
   * @brief Begins a render pass targeting the specified render target
   * @param renderTarget The render target to render into
   * @param clearColor Whether to clear color attachments
   * @param clearDepth Whether to clear depth attachment
   * @param clearStencil Whether to clear stencil attachment
   * @note Command buffer must be in Recording state
   * @note Transitions to InRenderPass state
   */
  virtual void beginRenderPass(const std::shared_ptr<RenderTarget> &renderTarget,
                               bool clearColor = true,
                               bool clearDepth = true,
                               bool clearStencil = false) = 0;

  /**
   * @brief Ends the current render pass
   * @note Command buffer must be in InRenderPass state
   * @note Transitions to Recording state
   */
  virtual void endRenderPass() = 0;

  // Pipeline and Resource Binding

  /**
   * @brief Sets the active pipeline state for subsequent draw calls
   * @param pipelineState The pipeline state to bind
   * @note Command buffer must be in InRenderPass state
   */
  virtual void setPipelineState(const std::shared_ptr<PipelineState> &pipelineState) = 0;

  /**
   * @brief Binds a resource set to the specified binding slot
   * @param resourceSet The resource set containing shader resources
   * @param setIndex The binding slot index for the resource set
   * @note Command buffer must be in InRenderPass state
   * @note Resource set must be built before binding
   */
  virtual void bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex = 0) = 0;

  /**
   * @brief Binds a vertex buffer for vertex data input
   * @param vertexBuffer The vertex buffer containing vertex data
   * @param bindingIndex The vertex input binding index
   * @param offset Byte offset into the vertex buffer
   * @note Command buffer must be in InRenderPass state
   */
  virtual void bindVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer,
                                uint32 bindingIndex = 0,
                                uint64 offset = 0) = 0;

  /**
   * @brief Binds an index buffer for indexed rendering
   * @param indexBuffer The index buffer containing vertex indices
   * @param offset Byte offset into the index buffer
   * @note Command buffer must be in InRenderPass state
   */
  virtual void bindIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer, uint64 offset = 0) = 0;

  // Dynamic State

  /**
   * @brief Sets the viewport transformation parameters
   * @param viewport Viewport description defining screen coordinate mapping
   * @note Command buffer must be in InRenderPass state
   */
  virtual void setViewport(const ViewportDesc &viewport) = 0;

  /**
   * @brief Sets the scissor test rectangle dimensions
   * @param scissor Scissor rectangle specification
   * @note Command buffer must be in InRenderPass state
   * @note Scissor test must be enabled in the rasterizer state
   */
  virtual void setScissor(const ScissorDesc &scissor) = 0;

  /**
   * @brief Sets the primitive topology for vertex assembly
   * @param primitiveTopology How vertices should be assembled into primitives
   * @note Command buffer must be in InRenderPass state
   * @note This overrides the topology from the pipeline state for dynamic topology changes
   * @note Common topologies include triangle lists, strips, and point lists
   */
  virtual void setPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

  // Drawing Commands

  /**
   * @brief Records a non-indexed draw call
   * @param vertexCount Number of vertices to render
   * @param instanceCount Number of instances to render
   * @param firstVertex Offset into the vertex buffer to start rendering from
   * @param firstInstance First instance ID for instanced rendering
   * @note Command buffer must be in InRenderPass state
   * @note Pipeline state and vertex buffer must be bound
   */
  virtual void draw(uint32 vertexCount,
                    uint32 instanceCount = 1,
                    uint32 firstVertex = 0,
                    uint32 firstInstance = 0) = 0;

  /**
   * @brief Records an indexed draw call
   * @param indexCount Number of indices to render
   * @param instanceCount Number of instances to render
   * @param firstIndex Offset into the index buffer to start rendering from
   * @param vertexOffset Base vertex offset added to each index value
   * @param firstInstance First instance ID for instanced rendering
   * @note Command buffer must be in InRenderPass state
   * @note Pipeline state, vertex buffer, and index buffer must be bound
   */
  virtual void drawIndexed(uint32 indexCount,
                           uint32 instanceCount = 1,
                           uint32 firstIndex = 0,
                           int32 vertexOffset = 0,
                           uint32 firstInstance = 0) = 0;

  // Resource Operations (outside render pass)

  /**
   * @brief Copies data between GPU buffers
   * @param srcBuffer Source buffer to copy from
   * @param dstBuffer Destination buffer to copy to
   * @param srcOffset Byte offset in source buffer
   * @param dstOffset Byte offset in destination buffer
   * @param size Number of bytes to copy
   * @note Command buffer must be in Recording state (not in render pass)
   */
  virtual void copyBuffer(const std::shared_ptr<GpuBuffer> &srcBuffer,
                          const std::shared_ptr<GpuBuffer> &dstBuffer,
                          uint64 srcOffset,
                          uint64 dstOffset,
                          uint64 size) = 0;

  /**
   * @brief Copies data from buffer to texture
   * @param srcBuffer Source buffer containing image data
   * @param dstTexture Destination texture
   * @param bufferOffset Byte offset in source buffer
   * @param mipLevel Target mip level in texture
   * @param arrayLayer Target array layer in texture
   * @note Command buffer must be in Recording state (not in render pass)
   */
  virtual void copyBufferToTexture(const std::shared_ptr<GpuBuffer> &srcBuffer,
                                   const std::shared_ptr<Texture> &dstTexture,
                                   uint64 bufferOffset = 0,
                                   uint32 mipLevel = 0,
                                   uint32 arrayLayer = 0) = 0;

  /**
   * @brief Updates buffer data with immediate data
   * @param buffer Target buffer to update
   * @param offset Byte offset in buffer
   * @param size Number of bytes to update
   * @param data Pointer to source data
   * @param accessType Access pattern hint for optimization
   * @note Command buffer must be in Recording state (not in render pass)
   */
  virtual void updateBuffer(const std::shared_ptr<GpuBuffer> &buffer,
                            uint64 offset,
                            uint64 size,
                            const void *data,
                            AccessType accessType = AccessType::WriteOnly) = 0;

  // Memory Barriers and Synchronization

  /**
   * @brief Inserts a memory barrier for resource synchronization
   * @param srcStage Pipeline stages that must complete before barrier
   * @param dstStage Pipeline stages that must wait for barrier
   * @note Command buffer must be in Recording state
   * @note Required for proper resource state transitions in Vulkan
   */
  virtual void memoryBarrier(uint32 srcStage, uint32 dstStage) = 0;

  // State Queries

  /**
   * @brief Gets the current command buffer state
   * @return Current state of the command buffer
   */
  virtual CommandBufferState getState() const = 0;

  /**
   * @brief Checks if command buffer is currently recording
   * @return True if in Recording or InRenderPass state
   */
  virtual bool isRecording() const = 0;

  /**
   * @brief Checks if command buffer is in a render pass
   * @return True if in InRenderPass state
   */
  virtual bool isInRenderPass() const = 0;
};
;