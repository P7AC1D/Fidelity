#include "VertexBuffer.h"

#include <stdexcept>

#include "../RenderApi/GL/GL.hpp"

namespace Rendering 
{
static uint32 ActiveVertexBuffer = -1;

struct VertexAttrib
{
  VertexAttribType Type;
  uint32 Size;
  uint32 Offset;
  uint32 Index;

  VertexAttrib(VertexAttribType type, uint32 size, uint32 offset, uint32 index):
    Type(type),
    Size(size),
    Offset(offset),
    Index(index)
  {}
};

GLenum BufferUsageToGLenum(BufferUsage usage)
{
  switch (usage)
  {
  case BufferUsage::Static: return GL_STATIC_DRAW;
  case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
  default: throw std::runtime_error("Unsupported buffer usage.");
  }
}

VertexBuffer::VertexBuffer():
  _vertexAttributeCount(0),
  _vertexStrideBytes(0),
  _vertexOffsetBytes(0),
  _refreshVertexAttribs(true)
{
  GLCall(glGenVertexArrays(1, &_vao));
  GLCall(glGenBuffers(1, &_vbo));
}

VertexBuffer::~VertexBuffer()
{
  Unbind();
  GLCall(glDeleteVertexArrays(1, &_vao));
  GLCall(glDeleteBuffers(1, &_vbo));
}

void VertexBuffer::Apply()
{  
  if (_refreshVertexAttribs)
  {
    RefreshVertexAttribs();
    _refreshVertexAttribs = false;
  }
  Bind();
}

void VertexBuffer::PushVertexAttrib(VertexAttribType vertexDeclType)
{  
  uint32 componentSize(0);
  auto currentOffsetBytes(_vertexOffsetBytes);
  switch (vertexDeclType)
  {
    case VertexAttribType::Vec2:
    {
      _vertexStrideBytes += 8;
      _vertexOffsetBytes += 8;
      componentSize = 2;
      break;
    }
    case VertexAttribType::Vec3:
    {
      _vertexStrideBytes += 12;
      _vertexOffsetBytes += 12;
      componentSize = 3;
      break;
    }
    case VertexAttribType::Vec4:
    {
      _vertexStrideBytes += 16;
      _vertexOffsetBytes += 16;
      componentSize = 4;
      break;
    }
    default: return;
  }

  _vertexAttribs.emplace_back(vertexDeclType, componentSize, currentOffsetBytes, _vertexAttributeCount);
  _vertexAttributeCount++;
  _refreshVertexAttribs = true;
}

void VertexBuffer::ResetVertexAttribs()
{
  Bind();
  for (auto& vertexDecl : _vertexAttribs)
  {
    GLCall(glDisableVertexAttribArray(vertexDecl.Index));
  }
  Unbind();

  _vertexAttributeCount = 0;
  _vertexStrideBytes = 0;
  _vertexOffsetBytes = 0;
  _vertexAttribs.clear();
}

void VertexBuffer::Bind()
{
  if (_vao != ActiveVertexBuffer)
  {
    GLCall(glBindVertexArray(_vao));
    ActiveVertexBuffer = _vao;
  }
}

void VertexBuffer::Unbind()
{
  if (_vao == ActiveVertexBuffer)
  {
    GLCall(glBindVertexArray(0));
    ActiveVertexBuffer = -1;
  }
}

void VertexBuffer::UploadData(const void* dataPtr, int32 dataBytes, BufferUsage bufferUsage)
{
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
  GLCall(glBufferData(GL_ARRAY_BUFFER, dataBytes, dataPtr, BufferUsageToGLenum(bufferUsage)));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::RefreshVertexAttribs()
{
  Bind();
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
  for (auto& vertexAttrib : _vertexAttribs)
  {
    GLCall(glVertexAttribPointer(vertexAttrib.Index, vertexAttrib.Size, GL_FLOAT, GL_FALSE, _vertexAttributeCount > 0 ?_vertexStrideBytes : 0, reinterpret_cast<GLvoid*>(vertexAttrib.Offset)));
    GLCall(glEnableVertexAttribArray(vertexAttrib.Index));
  }
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
  Unbind();
}
}
