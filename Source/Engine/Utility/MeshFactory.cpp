#include "MeshFactory.h"

#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "../Rendering/Material.h"
#include "../Rendering/StaticMesh.h"

using namespace Rendering;

static std::vector<Vector3> s_cubePoints =
{
  Vector3(-0.5f, -0.5f, 0.5f),
  Vector3(0.5f, -0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, 0.5f),
  Vector3(0.5f,  -0.5f, 0.5f),
  Vector3(0.5f,  0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, 0.5f),

  Vector3(-0.5f, -0.5f, -0.5f),
  Vector3(-0.5f, -0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, -0.5f),
  Vector3(-0.5f,  -0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, -0.5f),

  Vector3(-0.5f, 0.5f, 0.5f),
  Vector3(0.5f, 0.5f, 0.5f),
  Vector3(-0.5f,  0.5f, -0.5f),
  Vector3(0.5f,  0.5f, 0.5f),
  Vector3(0.5f,  0.5f, -0.5f),
  Vector3(-0.5f,  0.5f, -0.5f),

  Vector3(0.5f, -0.5f, 0.5f),
  Vector3(0.5f, -0.5f, -0.5f),
  Vector3(0.5f,  0.5f, 0.5f),
  Vector3(0.5f,  -0.5f, -0.5f),
  Vector3(0.5f,  0.5f, -0.5f),
  Vector3(0.5f,  0.5f, 0.5f),

  Vector3(0.5f, -0.5f, -0.5f),
  Vector3(-0.5f, -0.5f, -0.5f),
  Vector3(0.5f,  0.5f, -0.5f),
  Vector3(-0.5f,  -0.5f, -0.5f),
  Vector3(-0.5f,  0.5f, -0.5f),
  Vector3(0.5f,  0.5f, -0.5f),

  Vector3(-0.5f, -0.5f, -0.5f),
  Vector3(0.5f, -0.5f, -0.5f),
  Vector3(-0.5f,  -0.5f, 0.5f),
  Vector3(0.5f,  -0.5f, -0.5f),
  Vector3(0.5f,  -0.5f, 0.5f),
  Vector3(-0.5f,  -0.5f, 0.5f),
};

static std::vector<Vector3> s_cubeNormals =
{
  Vector3(0.0f, 0.0f, 1.0f),
  Vector3(0.0f, 0.0f, 1.0f),
  Vector3(0.0f, 0.0f, 1.0f),
  Vector3(0.0f, 0.0f, 1.0f),
  Vector3(0.0f, 0.0f, 1.0f),
  Vector3(0.0f, 0.0f, 1.0f),

  Vector3(-1.0f, 0.0f, 0.0f),
  Vector3(-1.0f, 0.0f, 0.0f),
  Vector3(-1.0f, 0.0f, 0.0f),
  Vector3(-1.0f, 0.0f, 0.0f),
  Vector3(-1.0f, 0.0f, 0.0f),
  Vector3(-1.0f, 0.0f, 0.0f),

  Vector3(0.0f, 1.0f, 0.0f),
  Vector3(0.0f, 1.0f, 0.0f),
  Vector3(0.0f, 1.0f, 0.0f),
  Vector3(0.0f, 1.0f, 0.0f),
  Vector3(0.0f, 1.0f, 0.0f),
  Vector3(0.0f, 1.0f, 0.0f),

  Vector3(1.0f, 0.0f, 0.0f),
  Vector3(1.0f, 0.0f, 0.0f),
  Vector3(1.0f, 0.0f, 0.0f),
  Vector3(1.0f, 0.0f, 0.0f),
  Vector3(1.0f, 0.0f, 0.0f),
  Vector3(1.0f, 0.0f, 0.0f),

  Vector3(0.0f, 0.0f, -1.0f),
  Vector3(0.0f, 0.0f, -1.0f),
  Vector3(0.0f, 0.0f, -1.0f),
  Vector3(0.0f, 0.0f, -1.0f),
  Vector3(0.0f, 0.0f, -1.0f),
  Vector3(0.0f, 0.0f, -1.0f),

  Vector3(0.0f, -1.0f, 0.0f),
  Vector3(0.0f, -1.0f, 0.0f),
  Vector3(0.0f, -1.0f, 0.0f),
  Vector3(0.0f, -1.0f, 0.0f),
  Vector3(0.0f, -1.0f, 0.0f),
  Vector3(0.0f, -1.0f, 0.0f)
};

static std::vector<Vector2> s_cubeUvs =
{
  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),

  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),

  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),

  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),

  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),

  Vector2(0.0f, 0.0f),
  Vector2(1.0f, 0.0f),
  Vector2(0.0f, 1.0f),
  Vector2(1.0f, 0.0f),
  Vector2(1.0f, 1.0f),
  Vector2(0.0f, 1.0f),
};

std::shared_ptr<StaticMesh> MeshFactory::CreateCube()
{
  auto mesh = std::make_shared<StaticMesh>("Cube");
  mesh->SetPositionVertexData(s_cubePoints);
  mesh->SetNormalVertexData(s_cubeNormals);
  mesh->SetTextureVertexData(s_cubeUvs);

  mesh->CalculateTangents(s_cubePoints, s_cubeUvs);
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
