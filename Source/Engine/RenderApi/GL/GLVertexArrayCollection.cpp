#include "GLVertexArrayCollection.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "../VertexLayout.hpp"
#include "GLVertexBuffer.hpp"
#include "GL.hpp"

GLenum GetComponentType(SemanticFormat format)
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

GLint GetComponentCount(SemanticFormat format)
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

GLint GetComponentByteCount(SemanticFormat format)
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

bool GLVertexArrayObject::operator==(const GLVertexArrayObject& vao) const
{
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
		if (_boundBuffers[i] == nullptr && vao._boundBuffers[i] == nullptr)
		{
			continue;
		}

		if (_boundBuffers[i] != nullptr && vao._boundBuffers[i] == nullptr)
		{
			return false;
		}

		if (_boundBuffers[i] == nullptr && vao._boundBuffers[i] != nullptr)
		{
			return false;
		}

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
		if (vao->_boundBuffers[i])
		{
			::Hash::Combine(seed, vao->_boundBuffers[i]->GetId());
		}    
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

GLVertexArrayObject::GLVertexArrayObject(uint32 vaoId, uint32 vsId, const std::array<std::shared_ptr<GLVertexBuffer>, MaxVertexBuffers>& boundBuffers):
  _vaoId((vaoId)),
  _vsId(vsId),
  _boundBuffers(boundBuffers)
{
}

const std::shared_ptr<GLVertexArrayObject> GLVertexArrayObjectCollection::GetVao(uint32 vertexShaderId,
                                                                                 const std::shared_ptr<VertexLayout>& vertexLayout,
                                                                                 const std::array<std::shared_ptr<GLVertexBuffer>, MaxVertexBuffers>& boundBuffers)
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
    stride += GetComponentByteCount(layouts[i].Format) * GetComponentCount(layouts[i].Format);
  }

	for (uint32 i = 0; i < boundBuffers.size(); i++)
	{
		if (boundBuffers[i])
		{
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, boundBuffers[i]->GetId()));
		}		
	}
  
  GLuint offset = 0;
  for (uint32 i = 0; i < layouts.size(); i++)
  {
    GLuint inputSlot = static_cast<GLuint>(layouts[i].Type);
    GLint compSize = GetComponentCount(layouts[i].Format);
    GLenum compType = GetComponentType(layouts[i].Format);
		GLboolean normalized = layouts[i].Normalised ? GL_TRUE : GL_FALSE;
        
    GLCall(glVertexAttribPointer(inputSlot, compSize, compType, normalized, stride, reinterpret_cast<GLvoid*>(offset)));
    GLCall(glEnableVertexAttribArray(inputSlot));

    offset += compSize * GetComponentByteCount(layouts[i].Format);
  }
  GLCall(glBindVertexArray(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
  
  expectedVAO->_vaoId = vao;
  _vaoBuffers.insert(expectedVAO);
  return expectedVAO;
}
