#include "UvSphere.hpp"

#include "../Maths/Math.hpp"

UvSphere::UvSphere(uint32 sectors, uint32 stacks) : _sectorCount(sectors),
                                                    _stackCount(stacks)
{
  GenerateVertices();
  GenerateIndices();
}

void UvSphere::GenerateVertices()
{
  float32 x, y, z, xy;
  float32 nx, ny, nz;
  float32 s, t;

  float32 sectorStep = Math::TwoPi / _sectorCount;
  float32 stackStep = Math::Pi / _stackCount;
  float32 sectorAngle, stackAngle;

  for (int32 i = 0; i <= _stackCount; i++)
  {
    stackAngle = Math::Pi / 2.0f - i * stackStep;
    xy = Math::Cos(stackAngle);
    z = Math::Sin(stackAngle);

    for (int32 j = 0; j <= _sectorCount; j++)
    {
      sectorAngle = j * sectorStep;

      x = xy * Math::Cos(sectorAngle);
      y = xy * Math::Sin(sectorAngle);
      _positions.emplace_back(x, y, z);

      s = (float32)j / _sectorCount;
      t = (float32)i / _stackCount;
      _texCoords.emplace_back(s, t);
    }
  }
}

void UvSphere::GenerateIndices()
{
  int32 k1, k2;
  for (int32 i = 0; i < _stackCount; i++)
  {
    k1 = i * (_sectorCount + 1);
    k2 = k1 + _sectorCount + 1;

    for (int32 j = 0; j < _sectorCount; j++, k1++, k2++)
    {
      if (i != 0)
      {
        _indices.push_back(k1);
        _indices.push_back(k2);
        _indices.push_back(k1 + 1);
      }

      if (i != (_stackCount - 1))
      {
        _indices.push_back(k1 + 1);
        _indices.push_back(k2);
        _indices.push_back(k2 + 1);
      }
    }
  }
}