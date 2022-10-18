#include "GLVertexArrayCollection.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "../VertexLayout.hpp"
#include "GLVertexBuffer.hpp"
#include "GL.hpp"

GLenum getComponentType(SemanticFormat format)
{
  switch (format)
  {
  case SemanticFormat::Byte:
  case SemanticFormat::Byte2:
  case SemanticFormat::Byte3:
  case SemanticFormat::Byte4:
    return GL_BYTE;
  case SemanticFormat::Ubyte:
  case SemanticFormat::Ubyte2:
  case SemanticFormat::Ubyte3:
  case SemanticFormat::Ubyte4:
    return GL_UNSIGNED_BYTE;
  case SemanticFormat::Float:
  case SemanticFormat::Float2:
  case SemanticFormat::Float3:
  case SemanticFormat::Float4:
    return GL_FLOAT;
  case SemanticFormat::Uint:
  case SemanticFormat::Uint2:
  case SemanticFormat::Uint3:
  case SemanticFormat::Uint4:
    return GL_UNSIGNED_INT;
  case SemanticFormat::Int:
  case SemanticFormat::Int2:
  case SemanticFormat::Int3:
  case SemanticFormat::Int4:
  default:
    return GL_INT;
  }
}

GLint getComponentCount(SemanticFormat format)
{
  switch (format)
  {
  case SemanticFormat::Byte:
  case SemanticFormat::Ubyte:
  case SemanticFormat::Float:
  case SemanticFormat::Uint:
  case SemanticFormat::Int:
    return 1;
  case SemanticFormat::Byte2:
  case SemanticFormat::Ubyte2:
  case SemanticFormat::Float2:
  case SemanticFormat::Uint2:
  case SemanticFormat::Int2:
    return 2;
  case SemanticFormat::Byte3:
  case SemanticFormat::Ubyte3:
  case SemanticFormat::Float3:
  case SemanticFormat::Uint3:
  case SemanticFormat::Int3:
    return 3;
  case SemanticFormat::Byte4:
  case SemanticFormat::Ubyte4:
  case SemanticFormat::Float4:
  case SemanticFormat::Uint4:
  case SemanticFormat::Int4:
  default:
    return 4;
  }
}

GLint getComponentByteCount(SemanticFormat format)
{
  switch (format)
  {
  case SemanticFormat::Byte:
  case SemanticFormat::Byte2:
  case SemanticFormat::Byte3:
  case SemanticFormat::Byte4:
  case SemanticFormat::Ubyte:
  case SemanticFormat::Ubyte2:
  case SemanticFormat::Ubyte3:
  case SemanticFormat::Ubyte4:
    return 1;
  case SemanticFormat::Uint:
  case SemanticFormat::Uint2:
  case SemanticFormat::Uint3:
  case SemanticFormat::Uint4:
  case SemanticFormat::Int:
  case SemanticFormat::Int2:
  case SemanticFormat::Int3:
  case SemanticFormat::Int4:
  case SemanticFormat::Float:
  case SemanticFormat::Float2:
  case SemanticFormat::Float3:
  case SemanticFormat::Float4:
  default:
    return 4;
  }
}

GLVertexArrayObject::GLVertexArrayObject() : _vaoId(0)
{
}

GLVertexArrayObject::GLVertexArrayObject(uint32 vaoId, const std::shared_ptr<GLVertexBuffer> &boundBuffer) : _vaoId((vaoId)),
                                                                                                             _boundBuffer(boundBuffer)
{
}

std::shared_ptr<GLVertexArrayObject> GLVertexArrayObjectCollection::getVao(const std::shared_ptr<VertexLayout> &vertexLayout, const std::shared_ptr<GLVertexBuffer> &boundBuffer)
{
  if (boundBuffer->_vao)
  {
    return boundBuffer->_vao;
  }

  GLuint vaoId = 0;
  glCall(glGenVertexArrays(1, &vaoId));
  ASSERT_TRUE(vaoId != 0, "Unabled to create OpenGL vertex array object");
  glCall(glBindVertexArray(vaoId));

  GLsizei stride = 0;
  auto layouts = vertexLayout->getDesc();
  for (uint32 i = 0; i < layouts.size(); i++)
  {
    stride += getComponentByteCount(layouts[i].Format) * getComponentCount(layouts[i].Format);
  }

  glCall(glBindBuffer(GL_ARRAY_BUFFER, boundBuffer->GetId()));

  GLuint offset = 0;
  for (uint32 i = 0; i < layouts.size(); i++)
  {
    GLuint inputSlot = static_cast<GLuint>(layouts[i].Type);
    GLint compSize = getComponentCount(layouts[i].Format);
    GLenum compType = getComponentType(layouts[i].Format);
    GLboolean normalized = layouts[i].Normalised ? GL_TRUE : GL_FALSE;

    glCall(glVertexAttribPointer(inputSlot, compSize, compType, normalized, stride, reinterpret_cast<GLvoid *>(offset)));
    glCall(glEnableVertexAttribArray(inputSlot));

    offset += compSize * getComponentByteCount(layouts[i].Format);
  }
  glCall(glBindVertexArray(0));
  glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

  std::shared_ptr<GLVertexArrayObject> vao(new GLVertexArrayObject(vaoId, boundBuffer));
  boundBuffer->_vao = vao;
  return vao;
}
