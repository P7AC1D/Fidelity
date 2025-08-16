#pragma once
#include <memory>

class RenderDevice;
class ICommandBuffer;

// Submit recorded command buffer via device's graphics queue when available,
// otherwise fall back to ICommandBuffer::execute().
void submitViaQueueOrExecute(const std::shared_ptr<RenderDevice> &renderDevice,
                             const std::unique_ptr<ICommandBuffer> &commandBuffer);
