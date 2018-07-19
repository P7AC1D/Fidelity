#include "Rectangle2I.hpp"

Rectangle2I::Rectangle2I() : X(0), Y(0), Width(0), Height(0)
{
}

Rectangle2I::Rectangle2I(int32 x, int32 y, uint32 width, uint32 height) : X(x), Y(y), Width(width), Height(height)
{
}

bool Rectangle2I::operator==(const Rectangle2I & rhs) const
{
	return X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height;
}

bool Rectangle2I::operator!=(const Rectangle2I & rhs) const
{
	return !(operator==(rhs));
}
