#include "Icosphere.hpp"

#include <unordered_map>

#include "../Maths/Math.hpp"

Icosphere::Icosphere(uint32 recursionCount) :
  _indexCount(0)
{
  GenerateIcoshedron();
  GenerateIcosphere(recursionCount);

  // Convert faces to indices
  _indices.reserve(3 * _faces.size());
  for (size_t i = 0; i < _faces.size(); i++)
  {
    _indices.push_back(_faces[i].indices[0]);
    _indices.push_back(_faces[i].indices[1]);
    _indices.push_back(_faces[i].indices[2]);
  }

  GenerateTexCoords();
}

uint32 Icosphere::AddVertex(const Vector3& vertex)
{
  _positions.push_back(Vector3::Normalize(vertex));
  return _indexCount++;
}

uint32 Icosphere::GetIndexedMidPoint(uint32 indexA, uint32 indexB)
{
  // Generate key
  bool isIndexASmaller = indexA < indexB;
  uint64 smallerIndex = isIndexASmaller ? indexA : indexB;
  uint64 largerIndex = isIndexASmaller ? indexB : indexA;
  uint64 key = (smallerIndex << 32) + largerIndex;

  auto iter = _midPointCache.find(key);
  if (iter != _midPointCache.end())
  {
    return iter->second;
  }

  // Not in cache, calculate it
  auto pointA = _positions[indexA];
  auto pointB = _positions[indexB];
  auto midPoint = (pointA + pointB) * 0.5f;

  uint32 midPointIndex = AddVertex(midPoint);
  _midPointCache.insert(std::make_pair(key, midPointIndex));
  return midPointIndex;
}

void Icosphere::GenerateIcosphere(uint32 recursionCount)
{
  // Refine faces to form icosphere
  for (uint32 i = 0; i < recursionCount; i++)
  {
    std::vector<TriangleFace> refinedFaces;
    for (size_t j = 0; j < _faces.size(); j++)
    {
      auto indexA = GetIndexedMidPoint(_faces[j].indices[0], _faces[j].indices[1]);
      auto indexB = GetIndexedMidPoint(_faces[j].indices[1], _faces[j].indices[2]);
      auto indexC = GetIndexedMidPoint(_faces[j].indices[2], _faces[j].indices[0]);

      refinedFaces.emplace_back(_faces[j].indices[0], indexA, indexC);
      refinedFaces.emplace_back(_faces[j].indices[1], indexB, indexA);
      refinedFaces.emplace_back(_faces[j].indices[2], indexC, indexB);
      refinedFaces.emplace_back(indexA, indexB, indexC);
    }
    _faces = refinedFaces;
  }
}

void Icosphere::GenerateIcoshedron()
{
  float32 t = (1.0f + std::sqrtf(5.0f)) / 2.0f;

  // Generate icosahedron vertices
  AddVertex(Vector3(-1.0f, t, 0.0f));
  AddVertex(Vector3(1.0f, t, 0.0f));
  AddVertex(Vector3(-1.0f, -t, 0.0f));
  AddVertex(Vector3(1.0f, -t, 0.0f));

  AddVertex(Vector3(0.0f, -1.0f, t));
  AddVertex(Vector3(0.0f, 1.0f, t));
  AddVertex(Vector3(0.0f, -1.0f, -t));
  AddVertex(Vector3(0.0f, 1.0f, -t));

  AddVertex(Vector3(t, 0.0f, -1.0f));
  AddVertex(Vector3(t, 0.0f, 1.0f));
  AddVertex(Vector3(-t, 0.0f, -1.0f));
  AddVertex(Vector3(-t, 0.0f, 1.0f));


  // Generate icosahedron _faces
  _faces.emplace_back(0, 11, 5);
  _faces.emplace_back(0, 5, 1);
  _faces.emplace_back(0, 1, 7);
  _faces.emplace_back(0, 7, 10);
  _faces.emplace_back(0, 10, 11);
  _faces.emplace_back(1, 5, 9);
  _faces.emplace_back(5, 11, 4);
  _faces.emplace_back(11, 10, 2);
  _faces.emplace_back(10, 7, 6);
  _faces.emplace_back(7, 1, 8);
  _faces.emplace_back(3, 9, 4);
  _faces.emplace_back(3, 4, 2);
  _faces.emplace_back(3, 2, 6);
  _faces.emplace_back(3, 6, 8);
  _faces.emplace_back(3, 8, 9);
  _faces.emplace_back(4, 9, 5);
  _faces.emplace_back(2, 4, 11);
  _faces.emplace_back(6, 2, 10);
  _faces.emplace_back(8, 6, 7);
  _faces.emplace_back(9, 8, 1);
}

void Icosphere::GenerateTexCoords()
{
  if (_positions.empty())
  {
    return;
  }

  _texCoords.reserve(_positions.size());
  for (size_t i = 0; i < _positions.size(); i++)
  {
    Vector3 d(Vector3::Normalize(-_positions[i]));
    float32 u = 0.5f + (std::atan2f(d[2], d[0]) / Math::TwoPi);
    float32 v = 0.5f - (std::asinf(d[1]) / Math::Pi);
    _texCoords.emplace_back(u, v);
  }

  auto wrappedIndices = GetWrappedTexCoords();
  FixWrappedTexCoords(wrappedIndices);
}

void Icosphere::FixWrappedTexCoords(const std::vector<uint32>& wrappedTexIndexIndices)
{
  uint32 vertexIndex = static_cast<uint32>(_positions.size()) - 1;
  std::unordered_map<uint32, uint32> visited;

  // For each problem vertex we need to create a copy of it, and add 1 to it’s U component. 
  // Then we need to add the new vertex copy to the mesh’ vertex array and keep a note of the index it was assigned.
  // This index replaces the corresponding index of the old vertex in the triangle.
  for (size_t i = 0; i < wrappedTexIndexIndices.size(); i += 3)
  {
    uint32 indexA = _indices[wrappedTexIndexIndices[i]];
    uint32 indexB = _indices[wrappedTexIndexIndices[i + 1]];
    uint32 indexC = _indices[wrappedTexIndexIndices[i + 2]];
    Vector2 texA = _texCoords[indexA];
    Vector2 texB = _texCoords[indexB];
    Vector2 texC = _texCoords[indexC];
    if (texA[0] < 0.25f)
    {
      auto iter = visited.find(indexA);
      if (iter == visited.end())
      {
        texA[0] += 1;
        _positions.push_back(_positions[indexA]);
        _texCoords.push_back(texA);
        vertexIndex++;
        visited[indexA] = vertexIndex;
        iter = visited.insert(std::make_pair(indexA, vertexIndex)).first;
      }
      indexA = iter->second;
    }

    if (texB[0] < 0.25f)
    {
      uint32 tempB = indexB;
      auto iter = visited.find(indexB);
      if (iter == visited.end())
      {
        texB[0] += 1;
        _positions.push_back(_positions[indexB]);
        _texCoords.push_back(texB);
        vertexIndex++;
        iter = visited.insert(std::make_pair(indexB, vertexIndex)).first;
      }
      indexB = iter->second;
    }

    if (texC[0] < 0.25f)
    {
      uint32 tempC = indexC;
      auto iter = visited.find(indexC);
      if (iter == visited.end())
      {
        texC[0] += 1;
        _positions.push_back(_positions[indexC]);
        _texCoords.push_back(texC);
        vertexIndex++;
        iter = visited.insert(std::make_pair(indexC, vertexIndex)).first;
      }
      indexC = iter->second;
    }
    _indices[wrappedTexIndexIndices[i]] = indexA;
    _indices[wrappedTexIndexIndices[i + 1]] = indexB;
    _indices[wrappedTexIndexIndices[i + 2]] = indexC;
  }
}

// Problem vertices will have a normal vector pointing inwards (-z);
std::vector<uint32> Icosphere::GetWrappedTexCoords()
{
  std::vector<uint32> indexIndices;
  for (size_t i = 0; i < _indices.size(); i += 3)
  {
    Vector3 texA(_texCoords[_indices[i]], 0.0f);
    Vector3 texB(_texCoords[_indices[i + 1]], 0.0f);
    Vector3 texC(_texCoords[_indices[i + 2]], 0.0f);
    Vector3 texNormal = Vector3::Cross(texC - texA, texB - texA);
    if (texNormal[2] < 0)
    {
      indexIndices.push_back(static_cast<uint32>(i));
      indexIndices.push_back(static_cast<uint32>(i + 1));
      indexIndices.push_back(static_cast<uint32>(i + 2));
    }
  }
  return indexIndices;
}