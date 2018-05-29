#include "GLVertexArrayCollection.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "../VertexLayout.hpp"
#include "GLVertexBuffer.hpp"
#include "GL.hpp"

GLuint GetInputSlot(SemanticType type)
{
  switch (type)
  {
    case SemanticType::Position: return 0;
    case SemanticType::Normal: return 1;
    case SemanticType::Tangent: return 2;
    case SemanticType::Bitangent: return 3;
    case SemanticType::TexCoord: return 4;
    default: return 0;
  }
}

GLenum GetComponentType(SemanticFormat format)
{
  switch (format)
  {
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

GLint GetComponentSize(SemanticFormat format)
{
  switch (format)
  {
    case SemanticFormat::Float:
    case SemanticFormat::Uint:
    case SemanticFormat::Int:
      return 1;
    case SemanticFormat::Float2:
    case SemanticFormat::Uint2:
    case SemanticFormat::Int2:
      return 2;
    case SemanticFormat::Float3:
    case SemanticFormat::Uint3:
    case SemanticFormat::Int3:
      return 3;
    case SemanticFormat::Float4:
    case SemanticFormat::Uint4:
    case SemanticFormat::Int4:
    default:
      return 4;
  }
}

bool GLVertexArrayObject::operator==(const GLVertexArrayObject& vao) const
{
  if (_vaoId != vao._vaoId)
  {
    return false;
  }
  
  if (_vsId != vao._vsId)
  {
    return false;
  }
  
  if (_boundBuffers.size() != vao._boundBuffers.size())
  {
    return false;
  }
  
  for (uint32 i = 0; i < _boundBuffers.size(); i++)
  {
    if (_boundBuffers[i]->GetId() != vao._boundBuffers[i]->GetId())
    {
      return false;
    }
  }
  return true;
}

bool GLVertexArrayObject::operator!=(const GLVertexArrayObject& vao) const
{
  return !(operator==(vao));
}

std::size_t GLVertexArrayObject::Hash::operator()(const std::shared_ptr<GLVertexArrayObject>& vao) const
{
  std::size_t seed = 0;
  ::Hash::Combine(seed, vao->_vsId);
  for (uint32 i = 0; i < vao->_boundBuffers.size(); i++)
  {
    ::Hash::Combine(seed, vao->_boundBuffers[i]->GetId());
  }
  return seed;
}

bool GLVertexArrayObject::Equal::operator()(const std::shared_ptr<GLVertexArrayObject>& a, const std::shared_ptr<GLVertexArrayObject>& b) const
{
  return *a == *b;
}

GLVertexArrayObject::GLVertexArrayObject(): _vaoId(0), _vsId(0)
{
}

GLVertexArrayObject::GLVertexArrayObject(uint32 vaoId, uint32 vsId, const std::array<std::shared_ptr<GLVertexBuffer>, 32>& boundBuffers):
  _vaoId((vaoId)),
  _vsId(vsId),
  _boundBuffers(boundBuffers)
{
}

const std::shared_ptr<GLVertexArrayObject> GLVertexArrayObjectCollection::GetVao(uint32 vertexShaderId,
                                                                                 const std::shared_ptr<VertexLayout>& vertexLayout,
                                                                                 const std::array<std::shared_ptr<GLVertexBuffer>, 32>& boundBuffers)
{
  std::shared_ptr<GLVertexArrayObject> expectedVAO(new GLVertexArrayObject(0, vertexShaderId, boundBuffers));
  auto iter = _vaoBuffers.find(expectedVAO);
  if (iter != _vaoBuffers.end())
  {
    return *iter;
  }
  
  GLuint vao = 0;
  GLCall(glGenVertexArrays(1, &vao));
  Assert::ThrowIfFalse(vao != 0, "Unabled to create OpenGL vertex array object");
  GLCall(glBindVertexArray(vao));
  
  GLsizei stride = 0;
  auto layouts = vertexLayout->GetDesc();
  for (uint32 i = 0; i < layouts.size(); i++)
  {
    stride += 4 * GetComponentSize(layouts[i].Format);
  }
  
  GLuint offset = 0;
  for (uint32 i = 0; i < layouts.size(); i++)
  {
    GLuint inputSlot = GetInputSlot(layouts[i].Type);
    GLint compSize = GetComponentSize(layouts[i].Format);
    GLenum compType = GetComponentType(layouts[i].Format);
    offset += compSize * 4;
    
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, boundBuffers[inputSlot]->GetId()));
    GLCall(glVertexAttribPointer(inputSlot, compSize, compType, GL_FALSE, stride, reinterpret_cast<GLvoid*>(offset)));
    GLCall(glEnableVertexAttribArray(inputSlot));
  }
  GLCall(glBindVertexArray(0));
  
  expectedVAO->_vaoId = vao;
  _vaoBuffers.insert(expectedVAO);
  return expectedVAO;
}
