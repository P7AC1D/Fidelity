#pragma once
#include "Geometry.hpp"

class Cube : public Geometry
{
public:
  Cube();

private:
  void SetIndices();
};