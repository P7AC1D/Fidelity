#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer():
_vbo(0)
{
  glGenBuffers(1, &_vbo);
  glGenVertexArrays(1, &_vao);
}

VertexBuffer::~VertexBuffer()
{
  glDeleteBuffers(1, &_vbo);
  glDeleteVertexArrays(1, &_vao);
}

void VertexBuffer::Bind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
}

void VertexBuffer::Unbind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UploadData(uint32 sizeInBytes, VertexBufferUsage usage, const void* data)
{
  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GetBufferUsage(usage));
  glVertexAttribPointer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLenum VertexBuffer::GetBufferUsage(VertexBufferUsage usage)
{
  switch (usage)
  {
    case VertexBufferUsage::Static:
      return GL_STATIC_DRAW;
    case VertexBufferUsage::Dynamic:
      return GL_DYNAMIC_DRAW;
  }
}
