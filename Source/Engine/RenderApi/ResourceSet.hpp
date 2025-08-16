#pragma once
#include "Core/Types.hpp"
#include <memory>
#include <vector>

// Forward declarations
class RenderDevice;
class Texture;
class GpuBuffer;
class SamplerState;
class ImageView;

/**
 * @brief Enumeration of supported resource types that can be bound to shaders
 */
enum class ResourceType : uint8
{
  TEXTURE_2D,     ///< 2D texture resource
  TEXTURE_CUBE,   ///< Cube map texture resource
  UNIFORM_BUFFER, ///< Constant/uniform buffer for shader constants
  STORAGE_BUFFER, ///< Read/write storage buffer
  SAMPLER         ///< Texture sampling state
};

/**
 * @brief Describes a single resource binding within a resource set
 *
 * Used internally to track resource bindings before they are committed
 * to the underlying graphics API implementation.
 */
struct ResourceBinding
{
  ResourceType type;    ///< Type of resource being bound
  uint32 binding;       ///< Binding slot index in the shader
  void *resource;       ///< Pointer to the actual resource object
  uint32 arraySize = 1; ///< Number of array elements (for texture arrays)
};

/**
 * @brief Interface for a collection of GPU resources that can be bound together
 *
 * Resource sets provide an efficient way to bind multiple GPU resources (textures,
 * buffers, samplers) to shaders with a single API call.
 *
 * @note Resource sets must be built before they can be bound to the pipeline
 * @note Once built, resource sets are typically immutable for performance reasons
 */
class IResourceSet
{
public:
  virtual ~IResourceSet() = default;

  /**
   * @brief Add a 2D texture to the resource set
   * @param binding Shader binding slot index
   * @param texture Shared pointer to the texture resource
   * @note Must be called before build()
   */
  virtual void addTexture(uint32 binding, const sptr<Texture> &texture) = 0;
  // Bind an ImageView (subresource/aspect) when supported by backend.
  virtual void addImageView(uint32 binding, const sptr<ImageView> &view) = 0;

  /**
   * @brief Add a uniform/constant buffer to the resource set
   * @param binding Shader binding slot index
   * @param buffer Shared pointer to the uniform buffer
   * @note Must be called before build()
   */
  virtual void addUniformBuffer(uint32 binding, const sptr<GpuBuffer> &buffer) = 0;

  /**
   * @brief Add a storage buffer to the resource set
   * @param binding Shader binding slot index
   * @param buffer Shared pointer to the storage buffer
   * @note Must be called before build()
   */
  virtual void addStorageBuffer(uint32 binding, const sptr<GpuBuffer> &buffer) = 0;

  /**
   * @brief Add a sampler state to the resource set
   * @param binding Shader binding slot index
   * @param sampler Shared pointer to the sampler state
   * @note Must be called before build()
   */
  virtual void addSampler(uint32 binding, const sptr<SamplerState> &sampler) = 0;

  /**
   * @brief Add an array of textures to the resource set
   * @param binding Shader binding slot index
   * @param textures Vector of texture shared pointers
   * @note Must be called before build()
   */
  virtual void addTextureArray(uint32 binding, const std::vector<sptr<Texture>> &textures) = 0;

  /**
   * @brief Build the resource set for use with the specified render device
   * @param renderDevice The render device to create API-specific resources for
   * @return True if build was successful, false otherwise
   * @note Must be called after all resources are added and before binding
   * @note Can only be called once - subsequent calls should return false
   */
  virtual bool build(const sptr<RenderDevice> &renderDevice) = 0;

  /**
   * @brief Bind this resource set to the rendering pipeline
   * @param renderDevice The render device to bind resources on
   * @param setIndex The descriptor set index (for APIs that support multiple sets)
   * @note Resource set must be built before calling this method
   */
  virtual void bind(const sptr<RenderDevice> &renderDevice, uint32 setIndex = 0) const = 0;

  /**
   * @brief Check if the resource set has been built and is ready for binding
   * @return True if built, false if still accepting resource additions
   */
  virtual bool isBuilt() const = 0;

  /**
   * @brief Reset the resource set to allow rebuilding with different resources
   * @note This invalidates any previously built API-specific objects
   */
  virtual void reset() = 0;
};

/**
 * @brief Interface for defining the layout of a resource set
 *
 * Resource set layouts define the types and binding slots of resources that
 * a resource set will contain. This allows the graphics API to optimize
 * memory layout and validate compatibility between shaders and resource sets.
 *
 * The layout must be created before creating resource sets that use it.
 */
class IResourceSetLayout
{
public:
  virtual ~IResourceSetLayout() = default;

  /**
   * @brief Add a binding definition to the layout
   * @param binding Shader binding slot index
   * @param type Type of resource that will be bound at this slot
   * @param count Number of array elements (default 1 for single resources)
   * @note Must be called before build()
   */
  virtual void addBinding(uint32 binding, ResourceType type, uint32 count = 1) = 0;

  /**
   * @brief Build the layout for use with the specified render device
   * @param renderDevice The render device to create API-specific layout for
   * @return True if build was successful, false otherwise
   * @note Must be called after all bindings are defined
   */
  virtual bool build(const sptr<RenderDevice> &renderDevice) = 0;

  /**
   * @brief Check if the layout has been built
   * @return True if built, false if still accepting binding definitions
   */
  virtual bool isBuilt() const = 0;
};

/**
 * @brief Factory interface for creating resource sets and layouts
 *
 * The factory pattern allows the renderer to create API-specific implementations
 * of resource sets and layouts without coupling to concrete types.
 *
 * @note Factory implementations should be registered with the render device
 */
class IResourceSetFactory
{
public:
  virtual ~IResourceSetFactory() = default;

  /**
   * @brief Create a new resource set layout
   * @return Unique pointer to the created layout
   * @note The returned layout must be configured and built before use
   */
  virtual std::unique_ptr<IResourceSetLayout> createLayout() = 0;

  /**
   * @brief Create a new resource set using the specified layout
   * @param layout The layout that defines the structure of this resource set
   * @return Unique pointer to the created resource set
   * @note The layout must be built before creating resource sets from it
   * @note The returned resource set must be configured and built before use
   */
  virtual std::unique_ptr<IResourceSet> createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout) = 0;
};