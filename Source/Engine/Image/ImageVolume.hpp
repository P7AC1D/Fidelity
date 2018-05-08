#pragma once
#include "../Core/Types.hpp"

struct ImageVolume
{
  uint32 Left;
  uint32 Right;
  uint32 Top;
  uint32 Bottom;
  uint32 Back;
  uint32 Front;
  
  inline uint32 GetWidth() const { return Right - Left; }
  inline uint32 GetHeight() const { return Top - Bottom; }
  inline uint32 GetDepth() const { return Front - Back; }
  inline uint32 GetSize() const { return GetWidth() * GetHeight() * GetDepth(); }
};
