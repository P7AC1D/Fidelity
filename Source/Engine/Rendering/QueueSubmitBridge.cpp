#include "QueueSubmitBridge.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/Queue.hpp"
#include "../RenderApi/CommandBuffer.hpp"

void submitViaQueueOrExecute(const std::shared_ptr<RenderDevice> &renderDevice,
                             const std::unique_ptr<ICommandBuffer> &commandBuffer)
{
  if (!commandBuffer)
    return;
  auto queue = renderDevice ? renderDevice->getGraphicsQueue() : nullptr;
  if (queue)
  {
    std::vector<std::shared_ptr<ICommandBuffer>> buffers;
    buffers.emplace_back(commandBuffer.get(), [](ICommandBuffer *) {});
    SubmitInfo info;
    info.commandBuffers = &buffers;
    queue->submit(info);
  }
  else
  {
    commandBuffer->execute();
  }
}
