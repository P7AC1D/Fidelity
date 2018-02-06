#include "IndexBuffer.hpp"

#include "OpenGL.h"

namespace Rendering {
  IndexBuffer::IndexBuffer()
  {
    GLCall(glGenBuffers(1, &_ibo));
  }

  IndexBuffer::~IndexBuffer()
  {
    GLCall(glDeleteBuffers(1, &_ibo));
  }

  void IndexBuffer::Bind()
  {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo));
  }

  void IndexBuffer::UploadData(const std::vector<uint32>& indices)
  {
    Bind();
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices.size(), &indices[0], GL_STATIC_DRAW));
  }
}