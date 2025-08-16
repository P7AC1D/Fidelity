#pragma once
#include "../CommandPool.hpp"
#include <memory>

class GLRenderDevice;
class ICommandBuffer;

/**
 * @brief Simple command pool for the GL backend that allocates GLCommandBuffer.
 */
class GLCommandPoolStub : public ICommandPool
{
public:
  explicit GLCommandPoolStub(std::shared_ptr<GLRenderDevice> device) : _device(std::move(device)) {}
  std::shared_ptr<ICommandBuffer> allocate() override;
  void reset() override {}

private:
  std::shared_ptr<GLRenderDevice> _device;
};
