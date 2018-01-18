#pragma once
#include <memory>
#include <vector>

#include "../Components/Component.h"
#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "Material.h"

namespace Rendering
{
class VertexBuffer;

enum VertexDataFormat : int32
{
  Position = 1 << 0,
  Normal   = 1 << 1,
  Uv       = 1 << 2,
  Tangent  = 1 << 3,
  Bitanget = 1 << 4
};

class StaticMesh
{
public:
  StaticMesh(const std::string& meshName, std::shared_ptr<Rendering::Material> material = std::make_shared<Rendering::Material>());
  ~StaticMesh();

  void SetPositionVertexData(const std::vector<Vector3>& positionData);
  void SetNormalVertexData(const std::vector<Vector3>& normalData);
  void SetTextureVertexData(const std::vector<Vector2>& textureData);
  void SetIndexData(const std::vector<uint32>& indexData);

  void CalculateTangents(const std::vector<Vector3>& positionData, const std::vector<Vector2>& textureData);

  std::shared_ptr<Rendering::Material> GetMaterial();
  std::shared_ptr<Rendering::VertexBuffer> GetVertexData();

  bool IsInitialized() const { return _isInitialized; }

private:
  std::vector<float32> CreateRestructuredVertexDataArray(int32& stride) const;
  std::vector<float32> CreateVertexDataArray() const;

  std::string _name;
  std::shared_ptr<Rendering::Material> _material;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;
  std::vector<Vector3> _positionData;
  std::vector<Vector3> _normalData;
  std::vector<Vector3> _tangentData;
  std::vector<Vector3> _bitangentData;
  std::vector<Vector2> _textureData;
  std::vector<uint32> _indexData;
  int32 _vertexDataFormat;
  int32 _vertexCount;
  bool _isInitialized;

  friend class Renderer;
};
}
