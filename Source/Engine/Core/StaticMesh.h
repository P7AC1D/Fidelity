#pragma once
#include <memory>
#include <vector>

#include "Maths.h"
#include "Types.hpp"

class IndexBuffer;
class Material;
class VertexBuffer;
class RenderDevice;

class StaticMesh
{
public:
  StaticMesh();

  void setPositionVertexData(const std::vector<Vector3> &positionData);
  void setNormalVertexData(const std::vector<Vector3> &normalData);
  void setTextureVertexData(const std::vector<Vector2> &textureData);
  void setTangentVertexData(const std::vector<Vector3> &tangentData);
  void setBitangentVertexData(const std::vector<Vector3> &bitangentData);
  void setIndexData(const std::vector<uint32> &indexData);

  Aabb getAabb();

  uint32 getVertexCount() const { return _vertexCount; }
  uint32 getIndexCount() const { return _indexCount; }

  void calculateTangents(const std::vector<Vector3> &positionData, const std::vector<Vector2> &textureData);
  void generateTangents();
  void generateNormals();

  std::shared_ptr<VertexBuffer> getVertexData(std::shared_ptr<RenderDevice> renderDevice);
  std::shared_ptr<IndexBuffer> getIndexData(std::shared_ptr<RenderDevice> renderDevice);

  bool isInitialized() const { return _verticesNeedUpdate && _indicesNeedUpdate; }
  bool isIndexed() const { return _indexed; }

private:
  enum VertexDataFormat : int32
  {
    Position = 1 << 0,
    Normal = 1 << 1,
    Uv = 1 << 2,
    Tangent = 1 << 3,
    Bitanget = 1 << 4
  };

  void calculateAabb();

  std::vector<float32> createRestructuredVertexDataArray(int32 &stride) const;
  std::vector<float32> createVertexDataArray() const;
  void uploadVertexData(std::shared_ptr<RenderDevice> renderDevice);
  void uploadIndexData(std::shared_ptr<RenderDevice> renderDevice);

  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<VertexBuffer> _vertexBuffer;

  std::vector<Vector3> _positionData;
  std::vector<Vector3> _normalData;
  std::vector<Vector3> _tangentData;
  std::vector<Vector3> _bitangentData;
  std::vector<Vector2> _textureData;
  std::vector<uint32> _indexData;

  int32 _vertexDataFormat;
  int32 _vertexCount;
  int32 _indexCount;

  bool _verticesNeedUpdate;
  bool _indicesNeedUpdate;
  bool _indexed;

  Aabb _aabb;
};
