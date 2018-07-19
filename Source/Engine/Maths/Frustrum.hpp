#pragma once
#include <array>
#include "AABB.hpp"
#include "Matrix4.hpp"
#include "Plane.hpp"

class Frustrum
{
public:
	Frustrum();
	Frustrum(const std::array<Plane, 6>& planes);
	Frustrum(const Matrix4& projection);

	bool Intersects(const Aabb& box) const;

private:
	std::array<Plane, 6> _planes;
};