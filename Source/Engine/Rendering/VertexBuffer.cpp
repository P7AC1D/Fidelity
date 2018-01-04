#include "VertexBuffer.h"

namespace Rendering
{
VertexBuffer::VertexBuffer(BufferType bufferType) :
  _bufferType(bufferType)
{
  glGenVertexArrays(1, &_vaoId);
}

VertexBuffer::~VertexBuffer()
{
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &_vaoId);
}

void VertexBuffer::UploadData(const void* dataPtr, int32 dataBytes, uint32 bufferUsage)
{
  glBindVertexArray(_vaoId);
  uint32 vboId;
  auto bufferType = static_cast<int32>(_bufferType);
  glGenBuffers(1, &vboId);
  glBindBuffer(bufferType, vboId);
  glBufferData(bufferType, dataBytes, dataPtr, bufferUsage);
  glBindVertexArray(0);
}
}