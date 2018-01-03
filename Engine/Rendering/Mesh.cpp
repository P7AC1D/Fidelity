#include "Mesh.hpp"

#include "../Core/RenderingSystem.hpp"
#include "VertexBuffer.hpp"

Mesh::Mesh():
_vertexDeclaration(VertexDeclaration::Positions),
_needsUpdate(true)
{
  static uint32 idCount = 0;
  _id = idCount;
  idCount++;
  
  _vertexBuffer.reset(new VertexBuffer);
}

void Mesh::SetPositions(const std::vector<Vector3>& positions)
{
  _positions = positions;
  _needsUpdate = true;
}

void Mesh::SetNormals(const std::vector<Vector3>& normals)
{
  _normals = normals;
  _needsUpdate = true;
  _vertexDeclaration |= VertexDeclaration:: Normals;
}

void Mesh::SetTangents(const std::vector<Vector3>& tangents)
{
  _tangents = tangents;
  _needsUpdate = true;
  _vertexDeclaration |= VertexDeclaration::Tangents;
}

void Mesh::SetBitangents(const std::vector<Vector3>& bitangets)
{
  _bitangents = bitangets;
  _needsUpdate = true;
  _vertexDeclaration |= VertexDeclaration::Bitangets;
}

void Mesh::SetTextureCoords(const std::vector<Vector2>& textureCoords)
{
  _textureCoords = textureCoords;
  _needsUpdate = true;
  _vertexDeclaration |= VertexDeclaration::TextureCoords;
}

void Mesh::SetMaterial(const std::shared_ptr<Material> material)
{
  _material = material;
}

void Mesh::Draw()
{
  if (_needsUpdate)
  {
    UploadData();
    _needsUpdate = false;
  }
  
  uint32 vertexCount = static_cast<uint32>(_positions.size());
  if (vertexCount > 0)
  {
    
  }
}

void Mesh::UploadData()
{
  uint32 vertexCount = static_cast<uint32>(_positions.size());
  
  std::vector<float32> vertices;
  vertices.reserve(vertexCount * 5); // Worse case.
  
  for (uint32 i = 0; i < vertexCount; i++)
  {
    uint32 vertexStride = 3;
    vertices.push_back(_positions[i][0]);
    vertices.push_back(_positions[i + 1][1]);
    vertices.push_back(_positions[i + 2][2]);
    
    if (_vertexDeclaration & VertexDeclaration::Normals)
    {
      vertexStride += 3;
      vertices.push_back(_normals[i][0]);
      vertices.push_back(_normals[i][1]);
      vertices.push_back(_normals[i][2]);
    }
    
    if (_vertexDeclaration & VertexDeclaration::Tangents)
    {
      vertexStride += 3;
      vertices.push_back(_tangents[i][0]);
      vertices.push_back(_tangents[i][1]);
      vertices.push_back(_tangents[i][2]);
    }
    
    if (_vertexDeclaration & VertexDeclaration::Bitangets)
    {
      vertexStride += 3;
      vertices.push_back(_bitangents[i][0]);
      vertices.push_back(_bitangents[i][1]);
      vertices.push_back(_bitangents[i][2]);
    }
    
    if (_vertexDeclaration & VertexDeclaration::TextureCoords)
    {
      vertexStride += 2;
      vertices.push_back(_textureCoords[i][0]);
      vertices.push_back(_textureCoords[i][1]);
    }
  }
  vertices.shrink_to_fit();
  _vertexBuffer->UploadData(vertices, VertexBufferUsage::Static);
}
