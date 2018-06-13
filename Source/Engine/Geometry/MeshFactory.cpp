#include "MeshFactory.h"

#include <unordered_map>
#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.h"
#include "../Rendering/StaticMesh.h"
#include "Cube.hpp"
#include "Icosphere.hpp"

using namespace Rendering;

std::shared_ptr<StaticMesh> MeshFactory::CreateCube()
{
  Cube cube;
  auto mesh = std::make_shared<StaticMesh>("Cube");
  mesh->SetPositionVertexData(cube.GetPositions());
  //mesh->SetTextureVertexData(cube.GetTexCoords());
  mesh->SetIndexData(cube.GetIndices());
  //mesh->GenerateNormals();
  //mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreatePlane(uint32 density)
{
  std::vector<Vector3> positions;
  std::vector<Vector3> normals;
  std::vector<Vector2> texCoords;

  float32 interval = 1.0f / static_cast<float32>(density);
  for (float32 i = -1.0f; i < 1.0f; i += interval)
  {
    for (float32 j = -1.0f; j < 1.0f; j += interval)
    {
      positions.emplace_back(i, 0.0f, j + interval);
      positions.emplace_back(i + interval, 0.0f, j);
      positions.emplace_back(i, 0.0f, j);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      texCoords.emplace_back(1.0f, 0.0f);
      texCoords.emplace_back(0.0f, 1.0f);
      texCoords.emplace_back(0.0f, 0.0f);

      positions.emplace_back(i + interval, 0.0f, j + interval);
      positions.emplace_back(i + interval, 0.0f, j);
      positions.emplace_back(i, 0.0f, j + interval);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      normals.emplace_back(0.0f, 1.0f, 0.0f);
      texCoords.emplace_back(1.0f, 1.0f);
      texCoords.emplace_back(0.0f, 1.0f);
      texCoords.emplace_back(1.0f, 0.0f);
    }
  }
  auto mesh = std::make_shared<StaticMesh>("Plane");
  mesh->SetPositionVertexData(positions);
  mesh->SetNormalVertexData(normals);
  mesh->SetTextureVertexData(texCoords);
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreateIcosphere(uint32 recursionCount)
{
  Icosphere icosphere(recursionCount);
  auto mesh = std::make_shared<StaticMesh>("Icosphere");
  mesh->SetPositionVertexData(icosphere.GetPositions());  
  mesh->SetTextureVertexData(icosphere.GetTexCoords());
  mesh->SetIndexData(icosphere.GetIndices());
  mesh->GenerateNormals();
  mesh->GenerateTangents();
  return mesh;
}
