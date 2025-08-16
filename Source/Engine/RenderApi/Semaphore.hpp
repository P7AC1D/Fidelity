#pragma once
#include <memory>
#include "../Core/Types.hpp"

/**
 * @brief GPU-GPU synchronization primitive.
 *
 * Semaphores coordinate execution order between queues and submissions. Two
 * modes are supported by the abstraction:
 * - Binary: simple signaled/unsignaled state.
 * - Timeline: monotonically increasing counter with signal/wait values.
 */
class ISemaphore
{
public:
  virtual ~ISemaphore() = default;

  /** Returns true if this semaphore is timeline-based. */
  virtual bool isTimeline() const = 0;

  /** Signals the timeline semaphore to the specified value (no-op for binary). */
  virtual void signal(uint64 value) = 0;

  /** Waits until the timeline semaphore reaches the specified value (no-op for binary). */
  virtual void wait(uint64 value) = 0;
};
