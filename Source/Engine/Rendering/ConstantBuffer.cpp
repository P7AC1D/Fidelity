#include "ConstantBuffer.h"

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
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glDeleteBuffers(1, &_uboId);
}

void ConstantBuffer::AllocateMemory(int32 sizeBytes)
{
  Initialize();
  glBindBuffer(GL_UNIFORM_BUFFER, _uboId);
  glBufferData(GL_UNIFORM_BUFFER, sizeBytes, nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  _sizeBytes = sizeBytes;
}

void ConstantBuffer::Initialize()
{
  if (!_isInitialized)
  {
    glGenBuffers(1, &_uboId);
    _isInitialized = true;
  }
}

void ConstantBuffer::UploadData(int32 offsetBytes, int32 countBytes, const void* data)
{
  Initialize();
  glBindBuffer(GL_UNIFORM_BUFFER, _uboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetBytes, countBytes, data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
}