#include "Cube.hpp"

#include <vector>

static const std::vector<Vector3> POSITIONS =
    {
        Vector3(-1.0f, -1.0f, 1.0f),
        Vector3(1.0f, -1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, 1.0f),
        Vector3(1.0f, -1.0f, 1.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, 1.0f),

        Vector3(-1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, -1.0f),

        Vector3(-1.0f, 1.0f, 1.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(-1.0f, 1.0f, -1.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(1.0f, 1.0f, -1.0f),
        Vector3(-1.0f, 1.0f, -1.0f),

        Vector3(1.0f, -1.0f, 1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(1.0f, 1.0f, -1.0f),
        Vector3(1.0f, 1.0f, 1.0f),

        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, -1.0f),
        Vector3(1.0f, 1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, 1.0f, -1.0f),
        Vector3(1.0f, 1.0f, -1.0f),

        Vector3(-1.0f, -1.0f, -1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, 1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(1.0f, -1.0f, 1.0f),
        Vector3(-1.0f, -1.0f, 1.0f),
};

static const std::vector<Vector2> TEX_COORDS =
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
  _positions = POSITIONS;
  _texCoords = TEX_COORDS;
  setIndices();
}

void Cube::setIndices()
{
  _indices.reserve(POSITIONS.size());
  for (size_t i = 0; i < POSITIONS.size(); i++)
  {
    _indices.push_back(i);
  }
}