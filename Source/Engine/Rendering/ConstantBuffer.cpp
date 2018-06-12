#include "ConstantBuffer.h"

#include "../RenderApi/GL/GL.hpp"

namespace Rendering
{
ConstantBuffer::ConstantBuffer(int32 sizeBytes) :
  _uboId(0),
  _isInitialized(false)
{
  AllocateMemory(sizeBytes);
}

ConstantBuffer::~ConstantBuffer()
{
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
  GLCall(glDeleteBuffers(1, &_uboId));
}

void ConstantBuffer::AllocateMemory(int32 sizeBytes)
{
  Initialize();
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, _uboId));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeBytes, nullptr, GL_STATIC_DRAW));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

  _sizeBytes = sizeBytes;
}

void ConstantBuffer::Initialize()
{
  if (!_isInitialized)
  {
    GLCall(glGenBuffers(1, &_uboId));
    _isInitialized = true;
  }
}

void ConstantBuffer::UploadData(int32 offsetBytes, int32 countBytes, const void* data)
{
  Initialize();
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, _uboId));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, offsetBytes, countBytes, data));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
}