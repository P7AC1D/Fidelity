#include "GLCommandPool.hpp"
#include "GLRenderDevice.hpp"
#include "GLCommandBuffer.hpp"

std::shared_ptr<ICommandBuffer> GLCommandPoolStub::allocate()
{
  return std::shared_ptr<ICommandBuffer>(new GLCommandBuffer(_device));
}
