#pragma once
#include <memory>
#include "../Core/Types.hpp"

class ICommandBuffer;

/**
 * @brief Allocator and recycler for command buffers.
 *
 * Pools amortize command buffer creation costs and enable efficient reset/reuse
 * per frame. The lifetime rules mirror Vulkan/DX12 concepts but remain backend-agnostic.
 */
class ICommandPool
{
public:
  virtual ~ICommandPool() = default;

  /** Allocate a new command buffer owned by the pool. */
  virtual std::shared_ptr<ICommandBuffer> allocate() = 0;

  /** Reset the pool and all command buffers allocated from it for reuse. */
  virtual void reset() = 0;
};
