#pragma once
#include "Geometry.hpp"

class UvSphere : public Geometry
{
public:
  UvSphere(uint32 sectors, uint32 stacks);

private:
  void generateVertices();
  void generateIndices();

  uint32 _sectorCount, _stackCount;
};