#include "VertexBuffer.h"

#include <stdexcept>

#include "OpenGL.h"

namespace Rendering {
  static uint32 ActiveVertexBuffer = -1;

  GLenum BufferUsageToGLenum(BufferUsage usage)
  {
    switch (usage)
    {
    case BufferUsage::Static: return GL_STATIC_DRAW;
    case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
    default: throw std::runtime_error("Unsupported buffer usage.");
    }
  }

  VertexBuffer::VertexBuffer()
  {
    GLCall(glGenVertexArrays(1, &_vaoId));
  }

  VertexBuffer::~VertexBuffer()
  {
    Unbind();
    GLCall(glDeleteVertexArrays(1, &_vaoId));
  }

  void VertexBuffer::Bind() const
  {
    if (_vaoId != ActiveVertexBuffer)
    {
      GLCall(glBindVertexArray(_vaoId));
      ActiveVertexBuffer = _vaoId;
    }
  }

  void VertexBuffer::Unbind() const
  {
    if (_vaoId == ActiveVertexBuffer)
    {
      GLCall(glBindVertexArray(0));
      ActiveVertexBuffer = -1;
    }
  }

  void VertexBuffer::UploadData(const void* dataPtr, int32 dataBytes, BufferUsage bufferUsage)
  {
    Bind();
    uint32 vboId;
    GLCall(glGenBuffers(1, &vboId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vboId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, dataBytes, dataPtr, BufferUsageToGLenum(bufferUsage)));
    Unbind();
  }
}
