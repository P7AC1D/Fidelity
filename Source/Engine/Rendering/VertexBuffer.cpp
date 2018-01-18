#include "VertexBuffer.h"

namespace Rendering
{
VertexBuffer::VertexBuffer(BufferType bufferType) :
  _bufferType(bufferType)
{
  GLCall(glGenVertexArrays(1, &_vaoId));
}

VertexBuffer::~VertexBuffer()
{
  GLCall(glBindVertexArray(0);
  GLCall(glDeleteVertexArrays(1, &_vaoId));
}

void VertexBuffer::UploadData(const void* dataPtr, int32 dataBytes, uint32 bufferUsage)
{
  GLCall(glBindVertexArray(_vaoId));
  uint32 vboId;
  auto bufferType = static_cast<int32>(_bufferType));
  GLCall(glGenBuffers(1, &vboId));
  GLCall(glBindBuffer(bufferType, vboId));
  GLCall(glBufferData(bufferType, dataBytes, dataPtr, bufferUsage));
  GLCall(glBindVertexArray(0));
}
}