#include "MeshFactory.h"

#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "Material.h"
#include "StaticMesh.h"

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

namespace Rendering
{
std::shared_ptr<StaticMesh> MeshFactory::CreateCube()
{
  auto material = std::make_shared<Material>();
  auto mesh = std::make_shared<StaticMesh>(material);
  mesh->SetPositionVertexData(s_cubePoints);
  mesh->SetNormalVertexData(s_cubeNormals);
  mesh->SetTextureVertexData(s_cubeUvs);

  mesh->CalculateTangents(s_cubePoints, s_cubeUvs);
  return mesh;
}
}