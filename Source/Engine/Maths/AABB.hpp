#pragma once
#include "Vector3.hpp"

struct Aabb
{
	Vector3 GetMidPoint() { return Vector3(PositiveBounds.X - NegativeBounds.X, PositiveBounds.Y - NegativeBounds.Y, PositiveBounds.Z - NegativeBounds.Z); }

	Vector3 PositiveBounds = Vector3::Zero;
	Vector3 NegativeBounds = Vector3::Zero;
};