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
    TriangleFace(uint32 a, uint32 b, uint32 c) : indices{a, b, c}
    {
    }

    uint32 indices[3];
  };

  uint32 addVertex(const Vector3 &vertex);
  uint32 getIndexedMidPoint(uint32 indexA, uint32 indexB);

  void generateIcosphere(uint32 recursionCount);
  void generateIcoshedron();
  void generateTexCoords();

  void fixPoleTexCoords();
  void fixMeridianTexCoords();

  std::unordered_map<uint64, uint32> _midPointCache;
  std::vector<TriangleFace> _faces;
  uint32 _indexCount;
};