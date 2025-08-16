#pragma once
#include <memory>
#include <vector>
#include "../Core/Types.hpp"
#include "GpuBuffer.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "Barrier.hpp"
#include "ResourceState.hpp"
class IQueryPool;

// Forward declarations
// Legacy PipelineState removed.
class IResourceSet;
class VertexBuffer;
class IndexBuffer;
struct ViewportDesc;
struct ScissorDesc;
class Texture;
class GpuBuffer;
enum class PrimitiveTopology;
class PipelineLayout;
enum class PipelineBindPoint : uint32_t;
class IResourceSet;

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
 * @brief Abstract interface for recording GPU commands.
 *
 * Commands are recorded into a buffer and then submitted to a queue for
 * execution by the GPU. Implementations must enforce the state machine:
 * Initial -> Recording -> InRenderPass -> Recording -> Executable.
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
   * @note Execution is asynchronous — use synchronization for completion detection
   */
  virtual void execute() = 0;

  /**
   * @brief Resets command buffer to initial state
   * @note Command buffer must not be pending execution on GPU
   * @note Transitions to Initial state
   */
  virtual void reset() = 0;

  // Render Pass Management

  // New API: Begin a render pass using explicit begin info
  virtual void beginRenderPass(const RenderPassBeginInfo &beginInfo) = 0;

  // New API: Advance to next subpass
  virtual void nextSubpass() = 0;

  /**
   * @brief Ends the current render pass
   * @note Command buffer must be in InRenderPass state
   * @note Transitions to Recording state
   */
  virtual void endRenderPass() = 0;

  // Pipeline and Resource Binding

  // Legacy setPipelineState removed; bind explicit graphics/compute pipelines.

  // New pipeline binding APIs
  /**
   * @brief Binds a graphics pipeline object for subsequent draw calls.
   * @note Command buffer must be in InRenderPass state.
   */
  virtual void bindGraphicsPipeline(const std::shared_ptr<class GraphicsPipelineState> &pipeline) = 0;

  /**
   * @brief Binds a compute pipeline object for subsequent dispatch calls.
   */
  virtual void bindComputePipeline(const std::shared_ptr<class ComputePipelineState> &pipeline) = 0;

  /**
   * Legacy binding path kept for compatibility; delegates to bindDescriptorSets.
   */
  virtual void bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex = 0) = 0;

  /**
   * @brief Binds descriptor sets using a pipeline layout
   * @param bindPoint Graphics or Compute
   * @param layout Pipeline layout describing set layouts
   * @param firstSet Starting set index
   * @param sets Array of descriptor set handles (use IResourceSet for now)
   * @param setCount Number of sets
   * @param dynamicOffsets Optional array of dynamic offsets applied in order
   * @param dynamicOffsetCount Count of dynamic offsets
   */
  virtual void bindDescriptorSets(PipelineBindPoint bindPoint,
                                  const std::shared_ptr<PipelineLayout> &layout,
                                  uint32 firstSet,
                                  const std::vector<const IResourceSet *> &sets,
                                  const uint32 *dynamicOffsets = nullptr,
                                  uint32 dynamicOffsetCount = 0) = 0;

  /**
   * @brief Push small constants to shaders
   * @param layout Pipeline layout containing push constant ranges
   * @param stageMask Stage mask
   * @param offset Byte offset into push constant block
   * @param size Byte size
   * @param data Pointer to data to upload
   */
  virtual void pushConstants(const std::shared_ptr<PipelineLayout> &layout,
                             uint32 stageMask,
                             uint32 offset,
                             uint32 size,
                             const void *data) = 0;

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

  // Indirect Drawing
  /**
   * @brief Records an indirect non-indexed draw call
   * @param argsBuffer GPU buffer containing DrawArraysIndirectCommand at the given byte offset
   * @param offset Byte offset into argsBuffer where the command struct begins
   * @note Command buffer must be in InRenderPass state
   */
  virtual void drawIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset) = 0;

  /**
   * @brief Records an indirect indexed draw call
   * @param argsBuffer GPU buffer containing DrawElementsIndirectCommand at the given byte offset
   * @param offset Byte offset into argsBuffer where the command struct begins
   * @note Command buffer must be in InRenderPass state
   */
  virtual void drawIndexedIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset) = 0;

  // Compute Dispatch
  /**
   * @brief Dispatches compute workgroups of size (x,y,z).
   */
  virtual void dispatch(uint32 x, uint32 y, uint32 z) = 0;

  /**
   * @brief Dispatches compute via indirect arguments from a GPU buffer.
   */
  virtual void dispatchIndirect(const std::shared_ptr<GpuBuffer> &buffer, uint64 offset) = 0;

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

  // Texture Operations - outside render pass
  /**
   * @brief Copies an entire mip level from src to dst (same size/format expected)
   */
  virtual void copyTexture(const std::shared_ptr<Texture> &src,
                           const std::shared_ptr<Texture> &dst,
                           uint32 srcMipLevel = 0,
                           uint32 dstMipLevel = 0) = 0;

  /**
   * @brief Blits color from src to dst with optional linear filtering (scales if sizes differ)
   */
  virtual void blitTexture(const std::shared_ptr<Texture> &src,
                           const std::shared_ptr<Texture> &dst,
                           bool linearFilter = false,
                           uint32 srcMipLevel = 0,
                           uint32 dstMipLevel = 0) = 0;

  /**
   * @brief Resolves a multisampled color texture into a single-sample destination
   */
  virtual void resolveTexture(const std::shared_ptr<Texture> &srcMsaa,
                              const std::shared_ptr<Texture> &dstSingle,
                              uint32 srcMipLevel = 0,
                              uint32 dstMipLevel = 0) = 0;

  /**
   * @brief Generates mipmaps for a texture
   */
  virtual void generateMips(const std::shared_ptr<Texture> &texture) = 0;

  /**
   * @brief Blits a depth texture into the default framebuffer's depth buffer.
   */
  virtual void blitDepthToDefault(const std::shared_ptr<Texture> &srcDepth) = 0;

  // Memory Barriers and Synchronization

  /**
   * @brief Inserts a memory barrier for resource synchronization
   * @param srcStage Pipeline stages that must complete before barrier
   * @param dstStage Pipeline stages that must wait for barrier
   * @note Command buffer must be in Recording state
   * @note Required for proper resource state transitions in Vulkan
   */
  virtual void memoryBarrier(uint32 srcStage, uint32 dstStage) = 0;

  /**
   * @brief Inserts explicit pipeline and resource barriers
   * @param srcStages Source pipeline stages mask (PipelineStage::Bits)
   * @param dstStages Destination pipeline stages mask
   * @param memoryDeps Memory dependency mask (MemoryDeps::Bits)
   * @param bufferBarriers Array of buffer barriers
   * @param imageBarriers Array of image barriers
   */
  virtual void pipelineBarrier(uint32 srcStages,
                               uint32 dstStages,
                               uint32 memoryDeps,
                               const std::vector<BufferBarrier> &bufferBarriers,
                               const std::vector<ImageBarrier> &imageBarriers) = 0;

  /**
   * @brief Helper: transition a single image or view between states
   */
  virtual void transition(const std::shared_ptr<ImageView> &view,
                          ResourceState oldState,
                          ResourceState newState,
                          ImageAspect aspect = ImageAspect::Color) = 0;

  // Queries and Timestamps
  /**
   * @brief Writes a GPU timestamp into the query at the given index.
   */
  virtual void writeTimestamp(const std::shared_ptr<IQueryPool> &pool, uint32 index) = 0;

  /**
   * @brief Begins a scoped query (e.g., time elapsed or occlusion) at query index.
   */
  virtual void beginQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index) = 0;

  /**
   * @brief Ends the previously begun scoped query.
   */
  virtual void endQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index) = 0;

  /**
   * @brief Resolves query results into CPU memory (blocking read).
   * @param pool Query pool
   * @param first First query index
   * @param count Number of queries to resolve
   * @param dst Pointer to array where 64-bit results will be written
   */
  virtual void resolveQueryData(const std::shared_ptr<IQueryPool> &pool, uint32 first, uint32 count, uint64 *dst) = 0;

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