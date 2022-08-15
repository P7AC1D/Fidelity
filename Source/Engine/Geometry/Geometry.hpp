#pragma once
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"

class Geometry
{
public:
  virtual ~Geometry() {}

  inline const std::vector<Vector2> &GetTexCoords() const { return _texCoords; }
  inline const std::vector<Vector3> &GetPositions() const { return _positions; }
  inline const std::vector<Vector3> &GetNormals() const { return _normals; }
  inline const std::vector<uint32> &GetIndices() const { return _indices; }

protected:
  std::vector<Vector2> _texCoords;
  std::vector<Vector3> _positions;
  std::vector<Vector3> _normals;
  std::vector<uint32> _indices;
};