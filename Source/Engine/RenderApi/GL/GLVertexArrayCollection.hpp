#pragma once
#include <memory>
#include <unordered_map>
#include "../../Core/Types.hpp"

class GLVertexBuffer;
class VertexLayout;

class GLVertexArrayObject
{
  friend class GLVertexArrayObjectCollection;
  
public:
  bool operator==(const GLVertexArrayObject& vao) const;
  bool operator!=(const GLVertexArrayObject& vao) const;
  
  uint32 GetId() const { return _vaoId; }
  
private:
  GLVertexArrayObject();
  GLVertexArrayObject(uint32 vaoId, const std::shared_ptr<GLVertexBuffer>& boundBuffer);
  
private:
  uint32 _vaoId;
  std::shared_ptr<GLVertexBuffer> _boundBuffer;
};

class GLVertexArrayObjectCollection
{
public:
  std::shared_ptr<GLVertexArrayObject> GetVao(const std::shared_ptr<VertexLayout>& vertexLayout, const std::shared_ptr<GLVertexBuffer>& boundBuffer);
  
private:
	std::unordered_map<std::shared_ptr<GLVertexBuffer>, std::shared_ptr<GLVertexArrayObject>> _vaos;
};
