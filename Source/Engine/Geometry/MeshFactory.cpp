#include "MeshFactory.h"

#include <unordered_map>
#include <vector>

#include "../Core/Maths.h"
#include "../Rendering/Material.h"
#include "../Rendering/StaticMesh.h"
#include "Cube.hpp"
#include "UvSphere.hpp"
#include "Icosphere.hpp"

std::shared_ptr<StaticMesh> MeshFactory::createCube()
{
  Cube cube;
  auto mesh = std::make_shared<StaticMesh>();
  mesh->setPositionVertexData(cube.getPositions());
  mesh->setTextureVertexData(cube.getTexCoords());
  mesh->setIndexData(cube.getIndices());
  mesh->generateNormals();
  mesh->generateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::createPlane(uint32 density)
{
  float32 interval = 1.0f / static_cast<float32>(density);

  uint32 vertexWidth = density + 2;
  uint32 vertexCount = vertexWidth * vertexWidth;
  uint32 faceCount = (density + 1) * (density + 1);
  uint32 indexCount = faceCount * 6;

  std::vector<Vector3> positions;
  std::vector<Vector2> texCoords;
  std::vector<uint32> indices;

  positions.reserve(vertexCount);
  texCoords.reserve(vertexCount);
  indices.reserve(indexCount);

  for (uint32 i = 0; i < vertexWidth; i++)
  {
    float32 y = -1.0f + i * interval;
    for (uint32 j = 0; j < vertexWidth; j++)
    {
      float32 x = -1.0f + j * interval;
      positions.push_back(Vector3(x, 0.0f, y));
      texCoords.push_back(Vector2(i, j));
    }
  }

  uint32 offset = 0;
  for (uint32 i = 0; i < indexCount; i = i + 6)
  {
    uint32 cornerIndex = i / 6 + offset;
    if ((cornerIndex + 1) % vertexWidth == 0)
    {
      offset++;
      cornerIndex++;
    }

    indices.push_back(cornerIndex);
    indices.push_back(cornerIndex + vertexWidth);
    indices.push_back(cornerIndex + vertexWidth + 1);

    indices.push_back(cornerIndex);
    indices.push_back(cornerIndex + vertexWidth + 1);
    indices.push_back(cornerIndex + 1);
  }

  auto mesh = std::make_shared<StaticMesh>();
  mesh->setPositionVertexData(positions);
  mesh->setTextureVertexData(texCoords);
  mesh->setIndexData(indices);
  mesh->generateNormals();
  mesh->generateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::createUvSphere(uint32 verticals, uint32 horizontals)
{
  UvSphere usSphere(verticals, horizontals);
  auto mesh = std::make_shared<StaticMesh>();
  mesh->setPositionVertexData(usSphere.getPositions());
  mesh->setTextureVertexData(usSphere.getTexCoords());
  mesh->setIndexData(usSphere.getIndices());
  mesh->generateNormals();
  mesh->generateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::createIcosphere(uint32 recursionCount)
{
  Icosphere icosphere(recursionCount);
  auto mesh = std::make_shared<StaticMesh>();
  mesh->setPositionVertexData(icosphere.getPositions());
  mesh->setTextureVertexData(icosphere.getTexCoords());
  mesh->setIndexData(icosphere.getIndices());
  mesh->generateNormals();
  mesh->generateTangents();
  return mesh;
}
