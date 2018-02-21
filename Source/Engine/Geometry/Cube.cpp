#include "Cube.hpp"

#include <vector>

static const std::vector<Vector3> Positions =
{
  Vector3(-1.0f, -1.0f, 1.0f),
  Vector3(1.0f, -1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, 1.0f),
  Vector3(1.0f,  -1.0f, 1.0f),
  Vector3(1.0f,  1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, 1.0f),

  Vector3(-1.0f, -1.0f, -1.0f),
  Vector3(-1.0f, -1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, -1.0f),
  Vector3(-1.0f,  -1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, -1.0f),

  Vector3(-1.0f, 1.0f, 1.0f),
  Vector3(1.0f, 1.0f, 1.0f),
  Vector3(-1.0f,  1.0f, -1.0f),
  Vector3(1.0f,  1.0f, 1.0f),
  Vector3(1.0f,  1.0f, -1.0f),
  Vector3(-1.0f,  1.0f, -1.0f),

  Vector3(1.0f, -1.0f, 1.0f),
  Vector3(1.0f, -1.0f, -1.0f),
  Vector3(1.0f,  1.0f, 1.0f),
  Vector3(1.0f,  -1.0f, -1.0f),
  Vector3(1.0f,  1.0f, -1.0f),
  Vector3(1.0f,  1.0f, 1.0f),

  Vector3(1.0f, -1.0f, -1.0f),
  Vector3(-1.0f, -1.0f, -1.0f),
  Vector3(1.0f,  1.0f, -1.0f),
  Vector3(-1.0f,  -1.0f, -1.0f),
  Vector3(-1.0f,  1.0f, -1.0f),
  Vector3(1.0f,  1.0f, -1.0f),

  Vector3(-1.0f, -1.0f, -1.0f),
  Vector3(1.0f, -1.0f, -1.0f),
  Vector3(-1.0f,  -1.0f, 1.0f),
  Vector3(1.0f,  -1.0f, -1.0f),
  Vector3(1.0f,  -1.0f, 1.0f),
  Vector3(-1.0f,  -1.0f, 1.0f),
};

static const std::vector<Vector2> TexCoords =
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

Cube::Cube()
{
  _positions = Positions;
  _texCoords = TexCoords;
  SetIndices();
}

void Cube::SetIndices()
{
  _indices.reserve(Positions.size());
  for (size_t i = 0; i < Positions.size(); i++)
  {
    _indices.push_back(i);
  }
}