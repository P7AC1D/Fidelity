#include "Icosphere.hpp"

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

  GenerateNormals();
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
  if (_normals.empty())
  {
    return;
  }

  _texCoords.reserve(_positions.size());
  for (size_t i = 0; i < _positions.size(); i++)
  {
    Vector2 vec(Vector2::Normalize(Vector2(_positions[i][0], _positions[i][1])));
    float32 u = std::asinf(vec[0]) / Math::Pi + 0.5f;
    float32 v = std::asinf(vec[1]) / Math::Pi + 0.5f;
    _texCoords.emplace_back(u, v);
  }
}