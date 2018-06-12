#pragma once
#include <memory>
#include <vector>

#include "../Components/Component.h"
#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"

class IndexBuffer;
class RenderDevice;
class VertexBuffer;

namespace Rendering
{
  class Material;
}

class StaticMesh
{
  friend class Renderer;
  
public:
  StaticMesh(const std::string& meshName);
  ~StaticMesh();

  void SetPositionVertexData(const std::vector<Vector3>& positionData);
  void SetNormalVertexData(const std::vector<Vector3>& normalData);
  void SetTextureVertexData(const std::vector<Vector2>& textureData);
  void SetTangentVertexData(const std::vector<Vector3>& tangentData);
  void SetBitangentVertexData(const std::vector<Vector3>& bitangentData);
  void SetIndexData(const std::vector<uint32>& indexData);

  inline const std::vector<Vector3>& GetPositionVertexData() { return _positionData; }
  inline const std::vector<Vector3>& GetNormalVertexData() { return _normalData; }
  inline const std::vector<Vector3>& GetTangentVertexData() { return _tangentData; }
  inline const std::vector<Vector3>& GetBitangentVertexData() { return _bitangentData; }
  inline const std::vector<Vector2>& GetTextureVertexData() { return _textureData; }

  inline uint32 GetVertexCount() const { return _vertexCount; }
  inline uint32 GetIndexCount() const { return _indexCount; }

  void CalculateTangents(const std::vector<Vector3>& positionData, const std::vector<Vector2>& textureData);
  void GenerateTangents();
  void GenerateNormals();

  std::shared_ptr<Rendering::Material> GetMaterial();

  bool IsInitialized() const { return _verticesNeedUpdate && _indicesNeedUpdate; }
  bool IsIndexed() const { return _indexed; }

private:
  std::shared_ptr<VertexBuffer> GetVertexData(const std::shared_ptr<RenderDevice>& renderDevice);
  std::shared_ptr<IndexBuffer> GetIndexData(const std::shared_ptr<RenderDevice>& renderDevice);
  std::vector<float32> CreateRestructuredVertexDataArray(int32& stride) const;
  std::vector<float32> CreateVertexDataArray() const;
  void UploadVertexData(const std::shared_ptr<RenderDevice>& renderDevice);
  void UploadIndexData(const std::shared_ptr<RenderDevice>& renderDevice);
  void SetVertexAttribs();

private:
  enum VertexDataFormat : int32
  {
    Position = 1 << 0,
    Normal   = 1 << 1,
    Uv       = 1 << 2,
    Tangent  = 1 << 3,
    Bitanget = 1 << 4
  };

  std::string _name;
  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<Rendering::Material> _material;
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

  friend class Renderer;
};
