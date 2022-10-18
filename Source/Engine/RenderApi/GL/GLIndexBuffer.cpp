#include "GLIndexBuffer.hpp"

#include "GLGpuBuffer.hpp"

void GLIndexBuffer::writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType)
{
  _buffer->writeData(byteOffset, byteCount, src, accessType);
}

void GLIndexBuffer::readData(uint64 byteOffset, uint64 byteCount, void *dst)
{
  _buffer->readData(byteOffset, byteCount, dst);
}

void GLIndexBuffer::copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  _buffer->copyData(dst, srcByteOffset, dstByteOffset, byteCount);
}

uint32 GLIndexBuffer::getId() const
{
  return _buffer->GetId();
}

GLIndexBuffer::GLIndexBuffer(const IndexBufferDesc &desc) : IndexBuffer(desc),
                                                            _buffer(new GLGpuBuffer(getDesc()))
{
}
