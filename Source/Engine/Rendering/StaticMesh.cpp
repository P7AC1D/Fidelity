#include "StaticMesh.h"

#include "Material.h"
#include "Renderer.h"
#include "VertexBuffer.h"

namespace Rendering
{
StaticMesh::StaticMesh(std::shared_ptr<Rendering::Material> material) :
  _material(material),
  _vertexBuffer(nullptr),
  _vertexDataFormat(0),
  _vertexCount(0),
  _isInitialized(false),
  Component("StaticMesh")
{}

StaticMesh::~StaticMesh()
{}

void StaticMesh::SetPositionVertexData(const std::vector<Vector3>& positionData)
{
  auto vertexCount = static_cast<int32>(positionData.size());
  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _positionData = positionData;
  _vertexDataFormat |= VertexDataFormat::Position;
  _isInitialized = false;
}

void StaticMesh::SetNormalVertexData(const std::vector<Vector3>& normalData)
{
  auto vertexCount = static_cast<int32>(normalData.size());
  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _normalData = normalData;
  _vertexDataFormat |= VertexDataFormat::Normal;
  _isInitialized = false;
}

void StaticMesh::SetTextureVertexData(const std::vector<Vector2>& textureData)
{
  auto vertexCount = static_cast<int32>(textureData.size());
  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _textureData = textureData;
  _vertexDataFormat |= VertexDataFormat::Uv;
  _isInitialized = false;
}

std::shared_ptr<Rendering::Material> StaticMesh::GetMaterial()
{
  return _material;
}

std::shared_ptr<Rendering::VertexBuffer> StaticMesh::GetVertexData()
{
  if (!_isInitialized)
  {
    int32 stride = 0;
    _vertexBuffer.reset(new VertexBuffer);
    _vertexBuffer->UploadData(CreateRestructuredVertexDataArray(stride), BufferUsage::Static);

    Renderer::SetVertexAttribPointers(this, stride);

    _positionData.clear();
    _positionData.shrink_to_fit();
    _normalData.clear();
    _normalData.shrink_to_fit();
    _textureData.clear();
    _textureData.shrink_to_fit();

    _isInitialized = true;
  }
  return _vertexBuffer;
}

std::vector<float32> StaticMesh::CreateRestructuredVertexDataArray(int32& stride) const
{
  std::vector<float32> restructuredData = CreateVertexDataArray();
  if (_vertexDataFormat & VertexDataFormat::Position)
  {
    stride += 3 * sizeof(float32);
  }
  if (_vertexDataFormat & VertexDataFormat::Normal)
  {
    stride += 3 * sizeof(float32);
  }
  if (_vertexDataFormat & VertexDataFormat::Uv)
  {
    stride += 2 * sizeof(float32);
  }

  for (int i = 0; i < _vertexCount; ++i)
  {
    if (_vertexDataFormat & VertexDataFormat::Position)
    {
      restructuredData.push_back(_positionData[i][0]);
      restructuredData.push_back(_positionData[i][1]);
      restructuredData.push_back(_positionData[i][2]);
    }
    if (_vertexDataFormat & VertexDataFormat::Normal)
    {
      restructuredData.push_back(_normalData[i][0]);
      restructuredData.push_back(_normalData[i][1]);
      restructuredData.push_back(_normalData[i][2]);
    }
    if (_vertexDataFormat & VertexDataFormat::Uv)
    {
      restructuredData.push_back(_textureData[i][0]);
      restructuredData.push_back(_textureData[i][1]);
    }
  }
  restructuredData.shrink_to_fit();
  return std::move(restructuredData);
}

std::vector<float32> StaticMesh::CreateVertexDataArray() const
{
  decltype(_positionData.size()) dataSize = 0;
  if (_vertexDataFormat & VertexDataFormat::Position)
  {
    dataSize += _positionData.size() * 3;
  }
  if (_vertexDataFormat & VertexDataFormat::Normal)
  {
    dataSize += _normalData.size() * 3;
  }
  if (_vertexDataFormat & VertexDataFormat::Uv)
  {
    dataSize += _textureData.size() * 2;
  }
  std::vector<float32> vertexDataArray;
  vertexDataArray.reserve(dataSize);
  return std::move(vertexDataArray);
}
}