#pragma once
#include "BlendState.hpp"
#include "DepthStencilState.hpp"
#include "GpuBuffer.hpp"
#include "IndexBuffer.hpp"
#include "PrimitiveTopology.hpp"
#include "RasterizerState.hpp"
#include "ResourceSet.hpp"
#include "SamplerState.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"
#include "Query.hpp"
#include "PresentMode.hpp"
#include "Surface.hpp"

// Forward declarations
class ICommandBuffer;
class IQueue;
class IFence;
class ISemaphore;
class ICommandPool;

/**
 * @brief Defines the viewport transformation parameters for rendering
 *
 * The viewport defines the transformation from normalized device coordinates
 * to screen coordinates. This affects how 3D coordinates are mapped to pixels.
 */
struct ViewportDesc
{
  float32 TopLeftX = 0.0f; ///< X coordinate of the viewport's top-left corner in pixels
  float32 TopLeftY = 0.0f; ///< Y coordinate of the viewport's top-left corner in pixels
  float32 Width;           ///< Width of the viewport in pixels
  float32 Height;          ///< Height of the viewport in pixels
  float32 MinDepth = 0.0f; ///< Minimum depth value (typically 0.0f for standard depth range)
  float32 MaxDepth = 1.0f; ///< Maximum depth value (typically 1.0f for standard depth range)
};

/**
 * @brief Defines scissor test rectangle for pixel culling
 *
 * The scissor test discards pixels outside the specified rectangle.
 * Must be enabled in the rasterizer state to take effect.
 */
struct ScissorDesc
{
  uint32 X; ///< X coordinate of the scissor rectangle's top-left corner
  uint32 Y; ///< Y coordinate of the scissor rectangle's top-left corner
  uint32 W; ///< Width of the scissor rectangle in pixels
  uint32 H; ///< Height of the scissor rectangle in pixels
};

/**
 * @brief Configuration parameters for render device initialization
 *
 * These settings control the fundamental behavior of the rendering system
 * and are typically set once during application startup.
 */
struct RenderDeviceDesc
{
  uint32 FrameCount = 2;          ///< Number of frames in flight for double/triple buffering
  uint32 RenderWidth;             ///< Target render resolution width in pixels
  uint32 RenderHeight;            ///< Target render resolution height in pixels
  bool FullscreenEnabled = false; ///< Whether to initialize in fullscreen mode
  bool VsyncEnabled = false;      ///< Whether to enable vertical synchronization
};

/**
 * @brief Flags for specifying which render target components to clear
 *
 * These flags can be combined using bitwise OR operations to clear
 * multiple render target components in a single operation.
 */
enum RenderTargetType
{
  RTT_Colour = 1, ///< Clear color buffer(s)
  RTT_Depth = 2,  ///< Clear depth buffer
  RTT_Stencil = 4 ///< Clear stencil buffer
};

/**
 * @brief Abstract device for creating and managing GPU resources.
 *
 * RenderDevice exposes a unified, API-agnostic interface for resource creation,
 * pipeline configuration, and command submission. Implementations provide the
 * concrete interaction with the underlying graphics API while keeping callers
 * independent of backend specifics.
 */
class RenderDevice
{
public:
  /**
   * @brief Constructs a render device with the specified configuration
   * @param desc Configuration parameters for the render device
   */
  RenderDevice(const RenderDeviceDesc &desc) : _desc(desc) {}

  // Resource Creation Methods

  /**
   * @brief Creates a shader object from source code
   * @param desc Shader description containing type and source code
   * @return Shared pointer to the created shader object
   * @note The shader must be compiled before use in a pipeline state
   */
  virtual std::shared_ptr<Shader> createShader(const ShaderDesc &desc) = 0;

  /**
   * @brief Creates an index buffer for storing vertex indices
   * @param desc Index buffer description specifying size, format, and usage
   * @return Shared pointer to the created index buffer
   * @note Index buffers optimize vertex reuse in triangle meshes
   */
  virtual std::shared_ptr<IndexBuffer> createIndexBuffer(const IndexBufferDesc &desc) = 0;

  /**
   * @brief Creates a vertex buffer for storing vertex data
   * @param desc Vertex buffer description specifying size and usage patterns
   * @return Shared pointer to the created vertex buffer
   * @note Vertex data layout is defined separately through VertexLayout
   */
  virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const VertexBufferDesc &desc) = 0;

  /**
   * @brief Creates a texture resource for storing image data
   * @param desc Texture description specifying format, dimensions, and usage
   * @param gammaCorrected Whether the texture data should be treated as gamma-corrected
   * @return Shared pointer to the created texture
   * @note Textures can be used as shader resources or render targets depending on usage flags
   */
  virtual std::shared_ptr<Texture> createTexture(const TextureDesc &desc, bool gammaCorrected = false) = 0;

  // Legacy RenderTarget removed. Use textures + Framebuffer instead.

  /**
   * @brief Creates a generic GPU buffer for various purposes
   * @param desc Buffer description specifying size, type, and usage patterns
   * @return Shared pointer to the created GPU buffer
   * @note Can be used for constant buffers, storage buffers, or other GPU data
   */
  virtual std::shared_ptr<GpuBuffer> createGpuBuffer(const GpuBufferDesc &desc) = 0;

  /**
   * @brief Creates a sampler state for texture filtering and addressing
   * @param desc Sampler state description specifying filtering and wrap modes
   * @return Shared pointer to the created sampler state
   * @note Sampler states control how textures are filtered and addressed during sampling
   */
  virtual std::shared_ptr<SamplerState> createSamplerState(const SamplerStateDesc &desc) = 0;

  /**
   * @brief Creates a resource set layout defining shader resource bindings
   * @return Unique pointer to the created resource set layout
   * @note Must be built before creating resource sets that use this layout
   */
  virtual std::unique_ptr<IResourceSetLayout> createResourceSetLayout() = 0;

  /**
   * @brief Creates a resource set for binding multiple resources to shaders
   * @param layout The layout that defines the structure of this resource set
   * @return Unique pointer to the created resource set
   * @note Resource sets provide efficient binding of multiple shader resources
   */
  virtual std::unique_ptr<IResourceSet> createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout) = 0;

  /**
   * @brief Creates a command buffer for recording rendering commands
   * @return Unique pointer to the created command buffer
   */
  virtual std::unique_ptr<ICommandBuffer> createCommandBuffer() = 0;

  /**
   * @brief Create a query pool for timestamps/elapsed/samples.
   * @return nullptr by default; backends may override.
   */
  virtual std::shared_ptr<IQueryPool> createQueryPool(const QueryPoolDesc &desc)
  {
    (void)desc;
    return nullptr;
  }

  /** Create a presentation surface from a native window handle. */
  virtual std::shared_ptr<ISurface> createSurface(void *nativeWindowHandle)
  {
    (void)nativeWindowHandle;
    return nullptr;
  }
  /** Create a swapchain for a surface with the given description. */
  virtual std::shared_ptr<ISwapchain> createSwapchain(const std::shared_ptr<ISurface> &surface, const SwapchainDesc &desc)
  {
    (void)surface;
    (void)desc;
    return nullptr;
  }

  // Debug/Validation Utilities

  /**
   * @brief Begin a GPU debug marker region (no-op if unsupported)
   * @param label Null-terminated label for the marker
   */
  virtual void beginDebugMarker(const char *label) { (void)label; }

  /**
   * @brief Insert a single GPU debug marker (no-op if unsupported)
   * @param label Null-terminated label for the marker
   */
  virtual void insertDebugMarker(const char *label) { (void)label; }

  /**
   * @brief End the current GPU debug marker region (no-op if unsupported)
   */
  virtual void endDebugMarker() {}

  /**
   * @brief Log GPU capabilities and selected device properties once at startup (no-op by default)
   */
  virtual void logCapabilities() const {}

  // Submission and synchronization primitives (safe defaults)

  /**
   * @brief Get the default graphics queue (nullptr if not implemented)
   */
  virtual std::shared_ptr<IQueue> getGraphicsQueue() { return nullptr; }

  /**
   * @brief Create a fence (nullptr if not implemented)
   */
  virtual std::shared_ptr<IFence> createFence(bool signaled = false)
  {
    (void)signaled;
    return nullptr;
  }

  /**
   * @brief Create a semaphore (nullptr if not implemented)
   */
  virtual std::shared_ptr<ISemaphore> createSemaphore(bool timeline = false, uint64 initialValue = 0)
  {
    (void)timeline;
    (void)initialValue;
    return nullptr;
  }

  /**
   * @brief Create a command pool (nullptr if not implemented)
   */
  virtual std::shared_ptr<ICommandPool> createCommandPool() { return nullptr; }

  // State Query Methods

  /**
   * @brief Retrieves the current viewport configuration
   * @return Reference to the current viewport description
   */
  virtual const ViewportDesc &getViewport() const = 0;

  /**
   * @brief Retrieves the current scissor rectangle dimensions
   * @return Current scissor rectangle description
   */
  virtual ScissorDesc getScissorDimensions() const = 0;

  // Default State Object Creation

  /**
   * @brief Creates a blend state with the specified configuration
   * @param desc Blend state description
   * @return Shared pointer to the created blend state
   * @note Provides default implementation using protected constructor
   */
  virtual std::shared_ptr<BlendState> createBlendState(const BlendStateDesc &desc)
  {
    return std::shared_ptr<BlendState>(new BlendState(desc));
  }

  /**
   * @brief Creates a depth-stencil state with the specified configuration
   * @param desc Depth-stencil state description
   * @return Shared pointer to the created depth-stencil state
   * @note Provides default implementation using protected constructor
   */
  virtual std::shared_ptr<DepthStencilState> createDepthStencilState(const DepthStencilStateDesc &desc)
  {
    return std::shared_ptr<DepthStencilState>(new DepthStencilState(desc));
  }

  /**
   * @brief Creates a pipeline state with the specified configuration
   * @param desc Pipeline state description containing shaders and render states
   * @return Shared pointer to the created pipeline state
   * @note Provides default implementation using protected constructor
   */

  /**
   * @brief Creates a rasterizer state with the specified configuration
   * @param desc Rasterizer state description
   * @return Shared pointer to the created rasterizer state
   * @note Provides default implementation using protected constructor
   */
  virtual std::shared_ptr<RasterizerState> createRasterizerState(const RasterizerStateDesc &desc)
  {
    return std::shared_ptr<RasterizerState>(new RasterizerState(desc));
  }

  /**
   * @brief Creates a vertex layout with the specified vertex attributes
   * @param desc Vector of vertex layout descriptors defining vertex attributes
   * @return Shared pointer to the created vertex layout
   * @note Provides default implementation using protected constructor
   */
  virtual std::shared_ptr<VertexLayout> createVertexLayout(const std::vector<VertexLayoutDesc> &desc)
  {
    return std::shared_ptr<VertexLayout>(new VertexLayout(desc));
  }

  // Device Configuration Queries

  /**
   * @brief Gets the target render width configured for this device
   * @return Render width in pixels
   */
  uint32 getRenderWidth() const { return _desc.RenderWidth; }

  /**
   * @brief Gets the target render height configured for this device
   * @return Render height in pixels
   */
  uint32 getRenderHeight() const { return _desc.RenderHeight; }

protected:
  RenderDeviceDesc _desc; ///< Device configuration parameters
};