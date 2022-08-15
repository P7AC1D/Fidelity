#pragma once
#include "Geometry.hpp"

class UvSphere : public Geometry
{
public:
  UvSphere(uint32 sectors, uint32 stacks);

private:
  void GenerateVertices();
  void GenerateIndices();

  uint32 _sectorCount, _stackCount;
};