#include "Icosphere.hpp"

#include <algorithm>
#include <unordered_map>

#include "../Maths/Math.hpp"

Icosphere::Icosphere(uint32 recursionCount) : _indexCount(0)
{
  generateIcoshedron();
  generateIcosphere(recursionCount);

  // Convert faces to indices
  _indices.reserve(3 * _faces.size());
  for (size_t i = 0; i < _faces.size(); i++)
  {
    _indices.push_back(_faces[i].indices[0]);
    _indices.push_back(_faces[i].indices[1]);
    _indices.push_back(_faces[i].indices[2]);
  }

  generateTexCoords();
}

uint32 Icosphere::addVertex(const Vector3 &vertex)
{
  _positions.push_back(Vector3::Normalize(vertex));
  return _indexCount++;
}

uint32 Icosphere::getIndexedMidPoint(uint32 indexA, uint32 indexB)
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

  uint32 midPointIndex = addVertex(midPoint);
  _midPointCache.insert(std::make_pair(key, midPointIndex));
  return midPointIndex;
}

void Icosphere::generateIcosphere(uint32 recursionCount)
{
  // Refine faces to form icosphere
  for (uint32 i = 0; i < recursionCount; i++)
  {
    std::vector<TriangleFace> refinedFaces;
    for (size_t j = 0; j < _faces.size(); j++)
    {
      auto indexA = getIndexedMidPoint(_faces[j].indices[0], _faces[j].indices[1]);
      auto indexB = getIndexedMidPoint(_faces[j].indices[1], _faces[j].indices[2]);
      auto indexC = getIndexedMidPoint(_faces[j].indices[2], _faces[j].indices[0]);

      refinedFaces.emplace_back(_faces[j].indices[0], indexA, indexC);
      refinedFaces.emplace_back(_faces[j].indices[1], indexB, indexA);
      refinedFaces.emplace_back(_faces[j].indices[2], indexC, indexB);
      refinedFaces.emplace_back(indexA, indexB, indexC);
    }
    _faces = refinedFaces;
  }
}

void Icosphere::generateIcoshedron()
{
  addVertex(Vector3(0.000f, 1.000f, 0.000f));
  addVertex(Vector3(0.894f, 0.447f, 0.000f));
  addVertex(Vector3(0.276f, 0.447f, 0.851f));
  addVertex(Vector3(-0.724f, 0.447f, 0.526f));

  addVertex(Vector3(-0.724f, 0.447f, -0.526f));
  addVertex(Vector3(0.276f, 0.447f, -0.851f));
  addVertex(Vector3(0.724f, -0.447f, 0.526f));
  addVertex(Vector3(-0.276f, -0.447f, 0.851f));

  addVertex(Vector3(-0.894f, -0.447f, 0.000f));
  addVertex(Vector3(-0.276f, -0.447f, -0.851f));
  addVertex(Vector3(0.724f, -0.447f, -0.526f));
  addVertex(Vector3(0.000f, -1.000f, 0.000f));

  _faces.emplace_back(11, 9, 10);
  _faces.emplace_back(11, 8, 9);
  _faces.emplace_back(11, 7, 8);
  _faces.emplace_back(11, 6, 7);
  _faces.emplace_back(11, 10, 6);
  _faces.emplace_back(0, 5, 4);
  _faces.emplace_back(0, 4, 3);
  _faces.emplace_back(0, 3, 2);
  _faces.emplace_back(0, 2, 1);
  _faces.emplace_back(0, 1, 5);
  _faces.emplace_back(10, 9, 5);
  _faces.emplace_back(9, 8, 4);
  _faces.emplace_back(8, 7, 3);
  _faces.emplace_back(7, 6, 2);
  _faces.emplace_back(6, 10, 1);
  _faces.emplace_back(5, 9, 4);
  _faces.emplace_back(4, 8, 3);
  _faces.emplace_back(3, 7, 2);
  _faces.emplace_back(2, 6, 1);
  _faces.emplace_back(1, 10, 5);
}

void Icosphere::generateTexCoords()
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

  fixMeridianTexCoords();
  fixPoleTexCoords();
}

void Icosphere::fixPoleTexCoords()
{
  auto northIter = std::find_if(_positions.begin(), _positions.end(), [](const Vector3 &vec)
                                { return 1.0f - vec[1] <= 0.000001f; });
  auto southIter = std::find_if(_positions.begin(), _positions.end(), [](const Vector3 &vec)
                                { return 1.0f - vec[1] <= 0.000001f; });
  Vector3 northVec = *northIter;
  Vector3 southVec = *southIter;
  auto northIndex = std::distance(_positions.begin(), northIter);
  auto southIndex = std::distance(_positions.begin(), southIter);

  uint32 vertexIndex = static_cast<uint32>(_positions.size() - 1);
  for (size_t i = 0; i < _indices.size(); i += 3)
  {
    if (_indices[i] == northIndex)
    {
      Vector2 texCoordB = _texCoords[_indices[i + 1]];
      Vector2 texCoordC = _texCoords[_indices[i + 2]];

      Vector2 newTexCoord(_texCoords[northIndex]);
      newTexCoord[0] = (texCoordB[0] + texCoordC[0]) / 2.0f;
      _texCoords.emplace_back(newTexCoord);

      Vector3 newVec(northVec);
      _positions.emplace_back(newVec);

      vertexIndex++;
      _indices[i] = vertexIndex;
    }
    else if (_indices[i] == southIndex)
    {
      Vector2 texCoordB = _texCoords[_indices[i + 1]];
      Vector2 texCoordC = _texCoords[_indices[i + 2]];

      Vector2 newTexCoord(_texCoords[northIndex]);
      newTexCoord[0] = (texCoordB[0] + texCoordC[0]) / 2.0f;
      _texCoords.emplace_back(newTexCoord);

      Vector3 newVec(southVec);
      _positions.emplace_back(newVec);

      vertexIndex++;
      _indices[i] = vertexIndex;
    }
  }
}

void Icosphere::fixMeridianTexCoords()
{
  // Problem vertices will have a normal vector pointing inwards (-z);
  std::vector<uint32> wrappedTexIndexIndices;
  for (size_t i = 0; i < _indices.size(); i += 3)
  {
    Vector3 texA(_texCoords[_indices[i]], 0.0f);
    Vector3 texB(_texCoords[_indices[i + 1]], 0.0f);
    Vector3 texC(_texCoords[_indices[i + 2]], 0.0f);
    Vector3 texNormal = Vector3::Cross(texC - texA, texB - texA);
    if (texNormal[2] < 0)
    {
      wrappedTexIndexIndices.push_back(static_cast<uint32>(i));
      wrappedTexIndexIndices.push_back(static_cast<uint32>(i + 1));
      wrappedTexIndexIndices.push_back(static_cast<uint32>(i + 2));
    }
  }

  uint32 vertexIndex = static_cast<uint32>(_positions.size()) - 1;
  std::unordered_map<uint32, uint32> visited;

  // For each problem vertex we need to create a copy of it, and add 1 to it�s U component.
  // Then we need to add the new vertex copy to the mesh� vertex array and keep a note of the index it was assigned.
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
        iter = visited.insert(std::make_pair(indexA, vertexIndex)).first;
      }
      indexA = iter->second;
    }

    if (texB[0] < 0.25f)
    {
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