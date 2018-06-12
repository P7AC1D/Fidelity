#pragma once
#include <array>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>
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
  class Hash
  {
  public:
    std::size_t operator()(const std::shared_ptr<GLVertexArrayObject>& vao) const;
  };
  
  class Equal
  {
  public:
    bool operator()(const std::shared_ptr<GLVertexArrayObject>& a, const std::shared_ptr<GLVertexArrayObject>& b) const;
  };
  
  GLVertexArrayObject();
  GLVertexArrayObject(uint32 vaoId, uint32 vsId, const std::array<std::shared_ptr<GLVertexBuffer>, 32>& boundBuffers);
  
private:
  uint32 _vaoId;
  uint32 _vsId;
  std::array<std::shared_ptr<GLVertexBuffer>, 32> _boundBuffers;
};

class GLVertexArrayObjectCollection
{
public:
  const std::shared_ptr<GLVertexArrayObject> GetVao(uint32 vertexShaderId, const std::shared_ptr<VertexLayout>& vertexLayout,
                                                    const std::array<std::shared_ptr<GLVertexBuffer>, 32>& boundBuffers);
  
private:
  std::unordered_set<std::shared_ptr<GLVertexArrayObject>, GLVertexArrayObject::Hash, GLVertexArrayObject::Equal> _vaoBuffers;
};
