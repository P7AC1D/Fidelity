#pragma once
#include <memory>
#include <vector>
#include "../Core/Types.hpp"

// Forward declarations to avoid circular includes
class ICommandBuffer;
class ISemaphore;

/**
 * @brief Type of hardware queue and the kinds of workloads it can execute.
 *
 * Not all backends expose distinct compute/transfer queues. When unavailable,
 * they may alias to the graphics queue.
 */
enum class QueueType : uint8
{
  Graphics,
  Compute,
  Transfer
};

/**
 * @brief Description of a semaphore and an optional timeline value.
 *
 * For timeline semaphores the value is used; for binary semaphores it is ignored.
 */
struct SubmitSemaphore
{
  std::shared_ptr<ISemaphore> semaphore; ///< Semaphore to wait on or signal
  uint64 value = 0;                      ///< Timeline value (ignored for binary)
};

/**
 * @brief Batch of work to submit to a queue.
 *
 * All arrays are optional; nullptr means no entries. Command buffers are
 * expected to be in an executable state. Backends should validate inputs.
 */
struct SubmitInfo
{
  const std::vector<std::shared_ptr<ICommandBuffer>> *commandBuffers = nullptr; ///< Command buffers to execute
  const std::vector<SubmitSemaphore> *waitSemaphores = nullptr;                 ///< Semaphores to wait before exec
  const std::vector<SubmitSemaphore> *signalSemaphores = nullptr;               ///< Semaphores to signal after exec
};

/**
 * @brief Abstraction of a GPU command queue.
 *
 * Submits recorded command buffers for execution and coordinates synchronization
 * via semaphores and fences.
 */
class IQueue
{
public:
  virtual ~IQueue() = default;

  /** Get the type/capabilities of this queue. */
  virtual QueueType getType() const = 0;

  /** Submit work to the queue. Implementations should be thread-safe. */
  virtual void submit(const SubmitInfo &info) = 0;

  /** Block until all previously submitted work to this queue has completed. */
  virtual void waitIdle() = 0;
};
