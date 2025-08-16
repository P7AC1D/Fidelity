#pragma once

/**
 * @brief Interface for accessing platform-specific GPU resource handles
 *
 * ResourceHandle provides a unified interface for obtaining native graphics API
 * handles from GPU resources. The exact handle type is backend-dependent.
 * This abstraction allows high-level rendering code to remain API-agnostic
 * while still providing access to native handles when needed.
 */
class ResourceHandle
{
public:
  virtual ~ResourceHandle() = default;

  /**
   * @brief Get the platform-specific native handle for this resource
   * @return Pointer to the native resource handle
   *
  * The returned pointer type depends on the underlying graphics API.
  * @note The caller is responsible for casting to the appropriate type
   * @note The lifetime of the returned handle is tied to this resource object
   */
  virtual void *getNativeHandle() const = 0;

  /**
   * @brief Check if the resource handle is valid and initialized
   * @return True if the resource has been successfully created and is ready for use
   *
   * A valid handle indicates that the underlying graphics API resource has been
   * created and is available for rendering operations. Invalid handles may occur
   * due to creation failures, device loss, or uninitialized resources.
   */
  virtual bool isValid() const = 0;
};