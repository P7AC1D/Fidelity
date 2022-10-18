#include "GLGpuBuffer.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLenum getBufferType(BufferType bufferType)
{
  switch (bufferType)
  {
  case BufferType::Vertex:
    return GL_ARRAY_BUFFER;
  case BufferType::Index:
    return GL_ELEMENT_ARRAY_BUFFER;
  case BufferType::Constant:
    return GL_UNIFORM_BUFFER;
  }
  throw std::runtime_error("Unsupported BufferType");
}

GLenum getBufferUsage(BufferUsage bufferUsage)
{
  switch (bufferUsage)
  {
  case BufferUsage::Default:
    return GL_STATIC_DRAW;
  case BufferUsage::Dynamic:
    return GL_DYNAMIC_DRAW;
  case BufferUsage::Stream:
    return GL_STREAM_DRAW;
  default:
    return GL_STATIC_DRAW;
  }
}

GLGpuBuffer::~GLGpuBuffer()
{
  if (_id != 0)
  {
    glCall(glDeleteBuffers(1, &_id));
  }
}

void GLGpuBuffer::writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType)
{
  void *dst = MapRange(byteOffset, byteCount, accessType);
  std::memcpy(dst, src, byteCount);
  Unmap();
}

void GLGpuBuffer::readData(uint64 byteOffset, uint64 byteCount, void *dst)
{
  void *src = MapRange(byteOffset, byteCount, AccessType::ReadOnly);
  std::memcpy(dst, src, byteCount);
  Unmap();
}

void GLGpuBuffer::copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  GLGpuBuffer *dstBuffer = static_cast<GLGpuBuffer *>(dst);
  GLGpuBuffer *srcBuffer = this;

  glCall(glBindBuffer(GL_COPY_READ_BUFFER, srcBuffer->_id));
  glCall(glBindBuffer(GL_COPY_WRITE_BUFFER, dstBuffer->_id));

  glCall(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcByteOffset, dstByteOffset, byteCount));

  glCall(glBindBuffer(GL_COPY_READ_BUFFER, 0));
  glCall(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
}

GLGpuBuffer::GLGpuBuffer(const GpuBufferDesc &desc) : GpuBuffer(desc), _id(0)
{
  Initialize();
}

void GLGpuBuffer::Initialize()
{
  if (isInitialized())
  {
    return;
  }

  glCall(glGenBuffers(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate buffer object");

  GLenum target = getBufferType(_desc.BufferType);
  glCall(glBindBuffer(target, _id));
  glCall(glBufferData(target, _desc.ByteCount, nullptr, getBufferUsage(_desc.BufferUsage)));

  _initialized = true;
}

void *GLGpuBuffer::MapRange(uint64 byteOffset, uint64 byteCount, AccessType accessType)
{
  ASSERT_FALSE(byteOffset + byteCount > _desc.ByteCount, "Mapped range cannot exceed the internal size of the buffer");

  GLenum target = getBufferType(_desc.BufferType);
  glCall(glBindBuffer(target, _id));

  GLenum access = 0;
  switch (accessType)
  {
  case AccessType::ReadOnly:
    access = GL_MAP_READ_BIT;
    break;
  case AccessType::WriteOnly:
    access = GL_MAP_WRITE_BIT;
    break;
  case AccessType::WriteOnlyDiscardRange:
    access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
    break;
  case AccessType::WriteOnlyDiscard:
    access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
    break;
  case AccessType::WriteOnlyUnsynchronized:
    access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
    break;
  default:
    access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
  }

  void *buffer = nullptr;

  glCall2(glMapBufferRange(target, byteOffset, byteCount, access), buffer);
  ASSERT_FALSE(buffer == nullptr, "Could not map buffer");
  return buffer;
}

void GLGpuBuffer::Unmap()
{
  GLenum target = getBufferType(_desc.BufferType);
  GLboolean success = GL_FALSE;
  glCall2(glUnmapBuffer(target), success);
  ASSERT_TRUE(success, "Buffer corrupt");
}
