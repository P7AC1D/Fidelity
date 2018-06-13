#include "GLVertexBuffer.hpp"

#include "GLGpuBuffer.hpp"

uint32 GLVertexBuffer::GetId() const
{
  return _buffer->GetId();
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType)
{
  _buffer->WriteData(byteOffset, byteCount, src, accessType);
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::ReadData(uint64 byteOffset, uint64 byteCount, void* dst)
{
  _buffer->ReadData(byteOffset, byteCount, dst);
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  _buffer->CopyData(dst, srcByteOffset, dstByteOffset, byteCount);
}

GLVertexBuffer::GLVertexBuffer(const VertexBufferDesc& desc):
  VertexBuffer(desc),
  _buffer(new GLGpuBuffer(GetDesc()))
{
}
