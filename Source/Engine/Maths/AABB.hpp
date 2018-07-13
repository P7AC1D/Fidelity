#pragma once
#include "Vector3.hpp"

struct Aabb
{
	Vector3 GetMidPoint() { return Vector3((PositiveBounds.X + NegativeBounds.X) / 2.0f, 
																				 (PositiveBounds.Y + NegativeBounds.Y) / 2.0f,
		(PositiveBounds.Z + NegativeBounds.Z) / 2.0f); }

	Vector3 PositiveBounds = Vector3::Zero;
	Vector3 NegativeBounds = Vector3::Zero;
};