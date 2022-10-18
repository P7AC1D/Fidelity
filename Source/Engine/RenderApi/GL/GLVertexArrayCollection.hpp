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
  uint32 getId() const { return _vaoId; }

private:
  GLVertexArrayObject();
  GLVertexArrayObject(uint32 vaoId, const std::shared_ptr<GLVertexBuffer> &boundBuffer);

private:
  uint32 _vaoId;
  std::shared_ptr<GLVertexBuffer> _boundBuffer;
};

class GLVertexArrayObjectCollection
{
public:
  static std::shared_ptr<GLVertexArrayObject> getVao(const std::shared_ptr<VertexLayout> &vertexLayout, const std::shared_ptr<GLVertexBuffer> &boundBuffer);
};
