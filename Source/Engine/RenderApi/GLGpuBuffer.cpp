#include "GLGpuBuffer.hpp"

#include "../Utility/Assert.hpp"
#include "GL.hpp"

GLGpuBuffer::GLGpuBuffer(const GpuBufferDesc& desc): _id(0)
{
  _desc = desc;
}

void GLGpuBuffer::Initialize()
{
  if (_id != 0)
  {
    return;
  }
  
  GLCall(glGenBuffers(1, &_id));
  Assert::ThrowIfTrue(_id == 0, "Could not generate buffer object");
  
}
