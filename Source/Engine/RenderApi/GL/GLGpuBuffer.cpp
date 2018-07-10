#include "GLGpuBuffer.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLenum GetBufferType(BufferType bufferType)
{
  switch (bufferType)
  {
    case BufferType::Vertex: return GL_ARRAY_BUFFER;
    case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::Constant: return GL_UNIFORM_BUFFER;
  }
  throw std::runtime_error("Unsupported BufferType");
}

GLenum GetBufferUsage(BufferUsage bufferUsage)
{
  switch (bufferUsage)
  {
    case BufferUsage::Default: return GL_STATIC_DRAW;
    case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
    default: return GL_STATIC_DRAW;
  }
}

GLGpuBuffer::~GLGpuBuffer()
{
  if (_id != 0)
  {
    GLCall(glDeleteBuffers(1, &_id));
  }
}

void GLGpuBuffer::WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType)
{
  void* dst = MapRange(byteOffset, byteCount, accessType);
  std::memcpy(dst, src, byteCount);
  Unmap();
}

void GLGpuBuffer::ReadData(uint64 byteOffset, uint64 byteCount, void* dst)
{
  void* src = MapRange(byteOffset, byteCount, AccessType::ReadOnly);
  std::memcpy(dst, src, byteCount);
  Unmap();
}

void GLGpuBuffer::CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount)
{
  GLGpuBuffer* dstBuffer = static_cast<GLGpuBuffer*>(dst);
  GLGpuBuffer* srcBuffer = this;

  GLCall(glBindBuffer(GL_COPY_READ_BUFFER, srcBuffer->_id));
  GLCall(glBindBuffer(GL_COPY_WRITE_BUFFER, dstBuffer->_id));

  GLCall(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcByteOffset, dstByteOffset, byteCount));

  GLCall(glBindBuffer(GL_COPY_READ_BUFFER, 0));
  GLCall(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
}

GLGpuBuffer::GLGpuBuffer(const GpuBufferDesc& desc): GpuBuffer(desc), _id(0)
{
  Initialize();
}

void GLGpuBuffer::Initialize()
{
  if (IsInitialized())
  {
    return;
  }
  
  GLCall(glGenBuffers(1, &_id));
  ASSERT_TRUE(_id == 0, "Could not generate buffer object");
  
  GLenum target = GetBufferType(_desc.BufferType);
  GLCall(glBindBuffer(target, _id));
  GLCall(glBufferData(target, _desc.ByteCount, nullptr, GetBufferUsage(_desc.BufferUsage)));

  _initialized = true;
}

void* GLGpuBuffer::MapRange(uint64 byteOffset, uint64 byteCount, AccessType accessType)
{
  ASSERT_TRUE(byteOffset + byteCount > _desc.ByteCount, "Mapped range cannot exceed the internal size of the buffer");

  GLenum target = GetBufferType(_desc.BufferType);
  GLCall(glBindBuffer(target, _id));

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
    case AccessType::WriteOnlyNoOverWrite:
      access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
      break;
    default:
      access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
  }

  void* buffer = nullptr;
	GLCall2(glMapBufferRange(target, byteOffset, byteCount, access), buffer);
  ASSERT_TRUE(buffer == nullptr, "Could not map buffer");
  return buffer;
}

void GLGpuBuffer::Unmap()
{
  GLenum target = GetBufferType(_desc.BufferType);
  GLboolean success = GL_FALSE;
	GLCall2(glUnmapBuffer(target), success);
  ASSERT_FALSE(success, "Buffer corrupt");
}
