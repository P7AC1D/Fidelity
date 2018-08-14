#include "GLIndexBuffer.hpp"

#include "GLGpuBuffer.hpp"

void GLIndexBuffer::WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType)
{
  _buffer->WriteData(byteOffset, byteCount, src, accessType);
}

void GLIndexBuffer::ReadData(uint64 byteOffset, uint64 byteCount, void* dst)
{
  _buffer->ReadData(byteOffset, byteCount, dst);
}

void GLIndexBuffer::CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  _buffer->CopyData(dst, srcByteOffset, dstByteOffset, byteCount);
}

uint32 GLIndexBuffer::GetId() const
{
  return _buffer->GetId();
}

GLIndexBuffer::GLIndexBuffer(const IndexBufferDesc& desc):
  IndexBuffer(desc),
  _buffer(new GLGpuBuffer(GetDesc()))
{
}
