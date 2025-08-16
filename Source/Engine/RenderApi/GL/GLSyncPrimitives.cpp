#include "GLSyncPrimitives.hpp"
#include "GLRenderDevice.hpp"
#include "GLCommandBuffer.hpp"

void GLQueue::submit(const SubmitInfo &info)
{
  // Wait on incoming semaphores (timeline only) for testing convenience.
  if (info.waitSemaphores)
  {
    for (const auto &w : *info.waitSemaphores)
    {
      if (w.semaphore && w.semaphore->isTimeline())
      {
        w.semaphore->wait(w.value);
      }
    }
  }

  if (info.commandBuffers)
  {
    for (const auto &cb : *info.commandBuffers)
    {
      if (cb)
      {
        cb->execute();
      }
    }
  }

  // Signal outgoing semaphores (timeline only)
  if (info.signalSemaphores)
  {
    for (const auto &s : *info.signalSemaphores)
    {
      if (s.semaphore && s.semaphore->isTimeline())
      {
        s.semaphore->signal(s.value);
      }
    }
  }
}
