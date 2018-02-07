#pragma once
#include <unordered_map>

#include "Geometry.hpp"

class Icosphere : public Geometry
{
public:
  Icosphere(uint32 recursionCount = 4);
  
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
  void GenerateTexCoords();
  void FixWrappedTexCoords(const std::vector<uint32>& indices);
  std::vector<uint32> GetWrappedTexCoords();

  std::unordered_map<uint64, uint32> _midPointCache;
  std::vector<TriangleFace> _faces;
  uint32 _indexCount;
};