#include "GLVertexBuffer.hpp"

#include "GLGpuBuffer.hpp"
#include "GLVertexArrayCollection.hpp"

uint32 GLVertexBuffer::GetId() const
{
  return _buffer->GetId();
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType)
{
  _buffer->writeData(byteOffset, byteCount, src, accessType);
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::readData(uint64 byteOffset, uint64 byteCount, void *dst)
{
  _buffer->readData(byteOffset, byteCount, dst);
}

// TODO: makes this more convienent for dealing with vertices
void GLVertexBuffer::copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  _buffer->copyData(dst, srcByteOffset, dstByteOffset, byteCount);
}

GLVertexBuffer::GLVertexBuffer(const VertexBufferDesc &desc) : VertexBuffer(desc),
                                                               _buffer(new GLGpuBuffer(getDesc()))
{
}
