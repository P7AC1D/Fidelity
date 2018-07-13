#include "StaticMesh.h"

#include "../RenderApi/IndexBuffer.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "Material.hpp"
#include "Renderer.h"

StaticMesh::StaticMesh() :
  _vertexDataFormat(0),
  _vertexCount(0),
	_verticesNeedUpdate(true),
	_indicesNeedUpdate(true),
  _indexed(false)
{}

void StaticMesh::SetPositionVertexData(const std::vector<Vector3>& positionData)
{
  auto vertexCount = static_cast<int32>(positionData.size());
  if (vertexCount == 0)
  {
		return;
  }

  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _positionData = positionData;
  _vertexDataFormat |= VertexDataFormat::Position;
	_verticesNeedUpdate = true;
}

void StaticMesh::SetNormalVertexData(const std::vector<Vector3>& normalData)
{
  auto vertexCount = static_cast<int32>(normalData.size());
  if (vertexCount == 0)
  {
    return;
  }

  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _normalData = normalData;
  _vertexDataFormat |= VertexDataFormat::Normal;
	_verticesNeedUpdate = true;
}
  
void StaticMesh::SetTangentVertexData(const std::vector<Vector3>& tangentData)
{
  auto vertexCount = static_cast<int32>(tangentData.size());
  if (vertexCount == 0)
  {
    return;
  }
  
  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;
  
  _tangentData = tangentData;
  _vertexDataFormat |= VertexDataFormat::Tangent;
	_verticesNeedUpdate = true;
}
  
void StaticMesh::SetBitangentVertexData(const std::vector<Vector3>& bitangentData)
{
  auto vertexCount = static_cast<int32>(bitangentData.size());
  if (vertexCount == 0)
  {
    return;
  }
  
  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;
  
  _bitangentData = bitangentData;
  _vertexDataFormat |= VertexDataFormat::Bitanget;
	_verticesNeedUpdate = true;
}

void StaticMesh::SetTextureVertexData(const std::vector<Vector2>& textureData)
{
  auto vertexCount = static_cast<int32>(textureData.size());
  if (vertexCount == 0)
  {
    return;
  }

  _vertexCount = _vertexCount >= vertexCount || _vertexCount == 0 ? vertexCount : _vertexCount;

  _textureData = textureData;
  _vertexDataFormat |= VertexDataFormat::Uv;
	_verticesNeedUpdate = true;
}

void StaticMesh::SetIndexData(const std::vector<uint32>& indexData)
{
  auto indexCount = static_cast<int32>(indexData.size());
  if (indexCount == 0)
  {
    _indexed = false;
    return;
  }
  _indexCount = indexCount;
  _indexData = indexData;
  _indicesNeedUpdate = true;
  _indexed = true;
}
  
void StaticMesh::GenerateTangents()
{
  if (_positionData.empty() && _textureData.empty())
  {
    return;
  }
  
  if (_indexed)
  {
    std::vector<Vector3> tangents(_positionData.size());
    std::vector<Vector3> bitangents(_positionData.size());
    
    // TODO: improve this using vectors instead of floats
    for (size_t i = 0; i < _indexData.size(); i += 3)
    {
      Vector3 p0 = _positionData[_indexData[i]];
      Vector3 p1 = _positionData[_indexData[i + 1]];
      Vector3 p2 = _positionData[_indexData[i + 2]];
      
      Vector2 uv0 = _textureData[_indexData[i]];
      Vector2 uv1 = _textureData[_indexData[i + 1]];
      Vector2 uv2 = _textureData[_indexData[i + 2]];
      
      Vector3 q0 = p1 - p0;
      Vector3 q1 = p2 - p0;

      Vector3 s = uv1 - uv0;
      Vector3 t = uv2 - uv0;
      
      Vector3 tangent;
      Vector3 bitangent;
      float32 demon = (s[0] * t[1] - t[0] * s[1]);
      if (std::fabsf(demon) >= 0e8f)
      {
        float32 f = 1.0f / demon;
        s *= f;
        t *= f;

        tangent = t[1] * q0 - s[1] * q1;
        bitangent = s[0] * q1 - t[0] * q0;
      }
      
      tangents[_indexData[i]] += tangent;
      tangents[_indexData[i + 1]] += tangent;
      tangents[_indexData[i + 2]] += tangent;
      
      bitangents[_indexData[i]] += bitangent;
      bitangents[_indexData[i + 1]] += bitangent;
      bitangents[_indexData[i + 2]] += bitangent;
    }
    
    for (size_t i = 0; i < _positionData.size(); i++)
    {
      tangents[i] = Vector3::Normalize(tangents[i]);
      bitangents[i] = Vector3::Normalize(bitangents[i]);
    }
    
    SetTangentVertexData(tangents);
    SetBitangentVertexData(bitangents);
  }
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
	_verticesNeedUpdate = true;
}

void StaticMesh::GenerateNormals()
{
  if (_positionData.empty())
  {
    return;
  }

  std::vector<Vector3> normals(_positionData.size());
  if (_indexed)
  {
    for (size_t i = 0; i < _indexData.size(); i += 3)
    {
      Vector3 vecA = _positionData[_indexData[i]];
      Vector3 vecB = _positionData[_indexData[i + 1]];
      Vector3 vecC = _positionData[_indexData[i + 2]];
      Vector3 vecAB = vecB - vecA;
      Vector3 vecAC = vecC - vecA;
      Vector3 normal = Vector3::Cross(vecAB, vecAC);
      normals[_indexData[i]] += normal;
      normals[_indexData[i + 1]] += normal;
      normals[_indexData[i + 2]] += normal;
    }

    for (size_t i = 0; i < normals.size(); i++)
    {
      normals[i] = Vector3::Normalize(normals[i]);
    }
  }
  else
  {    
    for (size_t i = 0; i < _positionData.size(); i += 3)
    {
      auto vecAB = _positionData[i + 1] - _positionData[i];
      auto vecAC = _positionData[i + 2] - _positionData[i];
      auto normal = Vector3::Normalize(Vector3::Cross(vecAB, vecAC));
      normals[i] = normal;
      normals[i + 1] = normal;
      normals[i + 2] = normal;
    }
  }
  SetNormalVertexData(normals);
}

std::shared_ptr<Material> StaticMesh::GetMaterial()
{
  return _material;
}

std::shared_ptr<VertexBuffer> StaticMesh::GetVertexData()
{
  if (_verticesNeedUpdate)
  {
    UploadVertexData();
		_verticesNeedUpdate = false;
  }
  return _vertexBuffer;
}

std::shared_ptr<IndexBuffer> StaticMesh::GetIndexData()
{
  if (_indicesNeedUpdate)
  {
    UploadIndexData();
		_indicesNeedUpdate = false;
  }
  return _indexBuffer;
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
  if (_vertexDataFormat & VertexDataFormat::Tangent)
  {
    stride += 3 * sizeof(float32);
  }
  if (_vertexDataFormat & VertexDataFormat::Bitanget)
  {
    stride += 3 * sizeof(float32);
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
  }
  restructuredData.shrink_to_fit();
  return restructuredData;
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
  if (_vertexDataFormat & VertexDataFormat::Tangent)
  {
    dataSize += _tangentData.size() * 3;
  }
  if (_vertexDataFormat & VertexDataFormat::Bitanget)
  {
    dataSize += _bitangentData.size() * 3;
  }
  std::vector<float32> vertexDataArray;
  vertexDataArray.reserve(dataSize);
  return vertexDataArray;
}

void StaticMesh::UploadVertexData()
{
  int32 stride = 0;
  auto dataToUpload = CreateRestructuredVertexDataArray(stride);
  
  VertexBufferDesc desc;
  desc.BufferUsage = BufferUsage::Default;
  desc.VertexCount = _vertexCount;
  desc.VertexSizeBytes = stride;
  _vertexBuffer = Renderer::GetRenderDevice()->CreateVertexBuffer(desc);
  _vertexBuffer->WriteData(0, dataToUpload.size() * sizeof(float32), dataToUpload.data(), AccessType::WriteOnlyDiscard);
}

void StaticMesh::UploadIndexData()
{
  IndexBufferDesc desc;
  desc.BufferUsage = BufferUsage::Default;
  desc.IndexCount = static_cast<uint32>(_indexData.size());
  desc.IndexType = IndexType::UInt32;
  _indexBuffer = Renderer::GetRenderDevice()->CreateIndexBuffer(desc);
  _indexBuffer->WriteData(0, _indexData.size() * IndexBuffer::GetBytesPerIndex(desc.IndexType), _indexData.data(), AccessType::WriteOnlyDiscard);
}
