#pragma once
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"

class Icosphere
{
public:
  Icosphere(uint32 recursionCount = 4);

  inline const std::vector<uint32>& GetIndices() const { return _indices; }
  inline const std::vector<Vector3>& GetVertexPositions() const { return _positions; }

private:
  struct TriangleFace
  {
    TriangleFace(uint32 a, uint32 b, uint32 c) :
      indices{ a, b, c }
    {
    }

    uint32 indices[3];
  };

  uint32 AddVertex(const Vector3& vertex);
  uint32 GetIndexedMidPoint(uint32 indexA, uint32 indexB);

  void GenerateIcosphere(uint32 recursionCount);
  void GenerateIcoshedron();

  std::unordered_map<uint64, uint32> _midPointCache;
  std::vector<uint32> _indices;
  std::vector<TriangleFace> _faces;
  std::vector<Vector3> _positions;
  uint32 _indexCount;
};