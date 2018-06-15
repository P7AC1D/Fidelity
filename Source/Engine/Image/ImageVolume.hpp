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

	ImageVolume():
		Left(0),
		Right(1),
		Bottom(0),
		Top(1),
		Back(0),
		Front(1)
	{}

	ImageVolume(uint32 left, uint32 right, uint32 bottom = 0, uint32 top = 1, uint32 back = 0, uint32 front = 1):
		Left(left),
		Right(right),
		Bottom(bottom),
		Top(top),
		Back(back),
		Front(front)
	{}
  
  inline uint32 GetWidth() const { return Right - Left; }
  inline uint32 GetHeight() const { return Top - Bottom; }
  inline uint32 GetDepth() const { return Front - Back; }
  inline uint32 GetSize() const { return GetWidth() * GetHeight() * GetDepth(); }
};
