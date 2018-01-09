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

void StaticMesh::CalculateTangents(const std::vector<Vector3>& positionData, const std::vector<Vector2>& textureData)
{
  _tangentData.reserve(positionData.size());
  _bitangentData.reserve(positionData.size());
  auto numFaces = positionData.size() / 3;
  for (auto i = 0; i < numFaces; i++)
  {
    Vector3 edge1 = positionData[i + 1] - positionData[i];
    Vector3 edge2 = positionData[i + 2] - positionData[i];
    Vector2 deltaUV1 = textureData[i + 1] - textureData[i];
    Vector2 deltaUV2 = textureData[i + 2] - textureData[i];

    Vector3 tangent;
    float32 f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
    tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
    tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
    tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);
    tangent = Vector3::Normalize(tangent);

    Vector3 bitangent;
    bitangent[0] = f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]);
    bitangent[1] = f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]);
    bitangent[2] = f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2]);
    bitangent = Vector3::Normalize(bitangent);

    _tangentData.push_back(tangent);
    _tangentData.push_back(tangent);
    _tangentData.push_back(tangent);
    _bitangentData.push_back(bitangent);
    _bitangentData.push_back(bitangent);
    _bitangentData.push_back(bitangent);
  }
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
    _tangentData.clear();
    _tangentData.shrink_to_fit();
    _bitangentData.clear();
    _bitangentData.shrink_to_fit();
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
  if (_vertexDataFormat & VertexDataFormat::Tangent)
  {
    stride += 3 * sizeof(float32);
  }
  if (_vertexDataFormat & VertexDataFormat::Bitanget)
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
    if (_vertexDataFormat & VertexDataFormat::Tangent)
    {
      restructuredData.push_back(_tangentData[i][0]);
      restructuredData.push_back(_tangentData[i][1]);
      restructuredData.push_back(_tangentData[i][2]);
    }
    if (_vertexDataFormat & VertexDataFormat::Bitanget)
    {
      restructuredData.push_back(_bitangentData[i][0]);
      restructuredData.push_back(_bitangentData[i][1]);
      restructuredData.push_back(_bitangentData[i][2]);
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
  if (_vertexDataFormat & VertexDataFormat::Tangent)
  {
    dataSize += _tangentData.size() * 3;
  }
  if (_vertexDataFormat & VertexDataFormat::Bitanget)
  {
    dataSize += _bitangentData.size() * 3;
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