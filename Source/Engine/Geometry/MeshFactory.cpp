#include "MeshFactory.h"

#include <unordered_map>
#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/MaterialFactory.hpp"
#include "../Rendering/StaticMesh.h"
#include "Cube.hpp"
#include "Icosphere.hpp"

std::shared_ptr<StaticMesh> MeshFactory::CreateCube()
{
  Cube cube;
  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(cube.GetPositions());
  mesh->SetTextureVertexData(cube.GetTexCoords());
  mesh->SetIndexData(cube.GetIndices());
	mesh->SetMaterial(MaterialFactory::Create());
  mesh->GenerateNormals();
  mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreatePlane(uint32 density)
{
  std::vector<Vector3> positions;
  std::vector<Vector2> texCoords;

  float32 interval = 1.0f / static_cast<float32>(density);
  //for (float32 i = -1.0f; i < 1.0f; i += interval)
  //{
  //  for (float32 j = -1.0f; j < 1.0f; j += interval)
  //  {
	float32 i = 0.0f;
	float32 j = 0.0f;
      positions.push_back(Vector3(i + interval, 0.0f, j));
      positions.push_back(Vector3(i, 0.0f, j + interval));
      positions.push_back(Vector3(i, 0.0f, j));
      texCoords.push_back(Vector2(1.0f, 0.0f));
      texCoords.push_back(Vector2(1.0f, 1.0f));
      texCoords.push_back(Vector2(0.0f, 0.0f));

      positions.push_back(Vector3(i + interval, 0.0f, j));
      positions.push_back(Vector3(i + interval, 0.0f, j + interval));
      positions.push_back(Vector3(i, 0.0f, j + interval));
      texCoords.push_back(Vector2(1.0f, 0.0f));
      texCoords.push_back(Vector2(1.0f, 1.0f));
      texCoords.push_back(Vector2(0.0f, 1.0f));
  //  }
  //}
  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(positions);
  mesh->SetTextureVertexData(texCoords);
	mesh->SetMaterial(MaterialFactory::Create());
	mesh->GenerateNormals();
	mesh->GenerateTangents();
  return mesh;
}

std::shared_ptr<StaticMesh> MeshFactory::CreateIcosphere(uint32 recursionCount)
{
  Icosphere icosphere(recursionCount);
  auto mesh = std::make_shared<StaticMesh>();
  mesh->SetPositionVertexData(icosphere.GetPositions());  
  mesh->SetTextureVertexData(icosphere.GetTexCoords());
  mesh->SetIndexData(icosphere.GetIndices());
	mesh->SetMaterial(MaterialFactory::Create());
  mesh->GenerateNormals();
  mesh->GenerateTangents();
  return mesh;
}
