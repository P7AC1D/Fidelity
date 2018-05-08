#include "GLRenderDevice.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLGpuBuffer.hpp"
#include "GLRenderTarget.hpp"

GLRenderDevice::GLRenderDevice(const RenderDeviceDesc& desc) :
  RenderDevice(desc),
  _primitiveTopology(PrimitiveTopology::TriangleList)
{
}

std::shared_ptr<GpuBuffer> GLRenderDevice::CreateGpuBuffer(const GpuBufferDesc& desc)
{
  return std::shared_ptr<GLGpuBuffer>(new GLGpuBuffer(desc));
}

std::shared_ptr<RenderTarget> GLRenderDevice::CreateRenderTarget(const RenderTargetDesc& desc)
{
  return std::shared_ptr<GLRenderTarget>(new GLRenderTarget(desc));
}

void GLRenderDevice::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
  _primitiveTopology = primitiveTopology;
}

void GLRenderDevice::SetViewport(const ViewportDesc& viewport)
{
  GLCall(glViewport(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height));
}

void GLRenderDevice::SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget)
{
  auto glRenderTarget = std::dynamic_pointer_cast<GLRenderTarget>(renderTarget);
  Assert::ThrowIfTrue(glRenderTarget == nullptr, "RenderTarget type mismatch");
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, glRenderTarget->GetId()));
}

void GLRenderDevice::Draw(uint32 vertexCount, uint32 vertexOffset)
{
  GLCall(glDrawArrays(GL_TRIANGLES, vertexOffset, vertexCount));
}

void GLRenderDevice::DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
{
  GLCall(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0));
}
