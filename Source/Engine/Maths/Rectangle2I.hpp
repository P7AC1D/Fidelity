#pragma once
#include "../Core/Types.hpp"

class Rectangle2I
{
public:
	Rectangle2I();
	Rectangle2I(int32 x, int32 y, uint32 width, uint32 height);

	bool operator==(const Rectangle2I& rhs) const;
	bool operator!=(const Rectangle2I& rhs) const;

public:
	int32 X, Y;
	uint32 Width, Height;
};