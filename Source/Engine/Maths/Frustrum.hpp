#pragma once
#include <array>
#include "AABB.hpp"
#include "Matrix4.hpp"
#include "Plane.hpp"
#include "Vector4.hpp"

class Camera;

class Frustrum
{
public:
	Frustrum();
	Frustrum(const Camera &camera);

	bool contains(const Aabb &box) const;

private:
	Plane _left;
	Plane _right;
	Plane _top;
	Plane _bottom;
	Plane _far;
	Plane _near;
};