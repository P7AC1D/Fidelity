#pragma once
#include <memory>
#include "../Core/Types.hpp"

/**
 * @brief CPU-visible completion primitive for GPU work.
 *
 * Fences are used to determine when submitted GPU commands have finished.
 * The interface mirrors common GPU APIs: a fence can be signaled by the GPU,
 * polled via isSignaled(), waited with an optional timeout, and reset for reuse.
 */
class IFence
{
public:
  virtual ~IFence() = default;

  /** Returns true if the fence is currently in the signaled state. */
  virtual bool isSignaled() const = 0;

  /**
   * Blocks the calling thread until the fence is signaled or the timeout expires.
   * @param timeoutNs Timeout in nanoseconds; ~0ull means wait indefinitely.
   */
  virtual void wait(uint64 timeoutNs = ~0ull) = 0;

  /** Resets the fence to the unsignaled state for reuse. */
  virtual void reset() = 0;
};
