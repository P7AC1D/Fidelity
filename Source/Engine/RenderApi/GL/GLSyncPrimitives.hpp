#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>
#include <chrono>
#include "../Fence.hpp"
#include "../Semaphore.hpp"
#include "../Queue.hpp"

class GLRenderDevice;
class GLCommandBuffer;

/**
 * @brief CPU-side fence for the GL backend.
 *
 * OpenGL 4.1 lacks cross-queue sync constructs matching modern APIs. This
 * implementation provides minimal CPU-visible signaling to support the
 * abstraction and tests without altering GL execution semantics.
 */
class GLFence : public IFence
{
public:
  explicit GLFence(bool signaled) : _signaled(signaled) {}
  bool isSignaled() const override { return _signaled.load(std::memory_order_acquire); }
  void wait(uint64 timeoutNs = ~0ull) override
  {
    if (isSignaled())
      return;
    std::unique_lock<std::mutex> lock(_mtx);
    if (timeoutNs == ~0ull)
    {
      _cv.wait(lock, [&]
               { return _signaled.load(std::memory_order_acquire); });
    }
    else
    {
      _cv.wait_for(lock, std::chrono::nanoseconds(timeoutNs), [&]
                   { return _signaled.load(std::memory_order_acquire); });
    }
  }
  void reset() override { _signaled.store(false, std::memory_order_release); }

  // Internal helper for GLQueue
  void signalInternal()
  {
    {
      std::lock_guard<std::mutex> lock(_mtx);
      _signaled.store(true, std::memory_order_release);
    }
    _cv.notify_all();
  }

private:
  std::atomic<bool> _signaled{false};
  mutable std::mutex _mtx;
  std::condition_variable _cv;
};

/**
 * @brief Binary/timeline semaphore stub for GL.
 *
 * For the GL backend this only updates CPU-visible state. It is sufficient
 * for ordering in tests and for future backends to map to native primitives.
 */
class GLSemaphore : public ISemaphore
{
public:
  explicit GLSemaphore(bool timeline, uint64 initial) : _isTimeline(timeline), _value(initial), _binary(false) {}
  explicit GLSemaphore() : _isTimeline(false), _value(0), _binary(false) {}

  bool isTimeline() const override { return _isTimeline; }

  void signal(uint64 value) override
  {
    if (_isTimeline)
    {
      std::lock_guard<std::mutex> lock(_mtx);
      if (value > _value)
        _value = value;
      _cv.notify_all();
    }
  }

  void wait(uint64 value) override
  {
    if (_isTimeline)
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _cv.wait(lock, [&]
               { return _value >= value; });
    }
  }

  // Binary path (unused in GL queue stub but kept for completeness)
  void setBinary(bool signaled)
  {
    std::lock_guard<std::mutex> lock(_mtx);
    _binary = signaled;
    _cv.notify_all();
  }
  void waitBinary()
  {
    std::unique_lock<std::mutex> lock(_mtx);
    _cv.wait(lock, [&]
             { return _binary; });
  }

private:
  bool _isTimeline;
  uint64 _value;
  bool _binary;
  mutable std::mutex _mtx;
  std::condition_variable _cv;
};

/**
 * @brief Immediate-mode queue stub for the GL backend.
 *
 * submit() executes command buffers immediately. Semaphores are honored only
 * via CPU-side waits/signals for testing convenience.
 */
class GLQueue : public IQueue
{
public:
  explicit GLQueue(std::shared_ptr<GLRenderDevice> device) : _device(std::move(device)) {}
  QueueType getType() const override { return QueueType::Graphics; }
  void submit(const SubmitInfo &info) override;
  void waitIdle() override {}

private:
  std::shared_ptr<GLRenderDevice> _device;
};
