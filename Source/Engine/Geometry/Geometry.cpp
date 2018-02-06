#include "Geometry.hpp"

Geometry::~Geometry()
{
}

void Geometry::GenerateNormals()
{
  if (_positions.empty())
  {
    return;
  }
  
  _normals.reserve(_positions.size());
  for (size_t i = 0; i < _positions.size(); i += 3)
  {
    auto vecAB = _positions[i + 1] - _positions[i];
    auto vecAC = _positions[i + 2] - _positions[i];
    auto normal = Vector3::Normalize(Vector3::Cross(vecAB, vecAC));
    _normals.push_back(normal);
    _normals.push_back(normal);
    _normals.push_back(normal);
  }
}