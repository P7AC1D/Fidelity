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
	Frustrum(const Matrix4& viewProjection);

	bool Intersects(const Aabb& box) const;

	std::array<Plane, 6> GetPlanes() const { return _planes; }

private:
	std::array<Plane, 6> _planes;
};