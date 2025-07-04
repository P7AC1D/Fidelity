#pragma once
#include <array>
#include "AABB.hpp"
#include "Matrix4.hpp"
#include "Plane.hpp"
#include "Vector4.hpp"

class Camera;
class Transform;

class Frustrum
{
public:
	Frustrum();
	Frustrum(const Camera &camera);

	bool contains(const Aabb &box, const Transform &transform) const;

private:
	// Helper methods for optimized culling
	bool containsAxisAligned(const Aabb &aabb, const Transform &transform) const;
	bool containsOriented(const Aabb &aabb, const Transform &transform) const;
	bool isTransformAxisAligned(const Transform &transform) const;

	Plane _left;
	Plane _right;
	Plane _top;
	Plane _bottom;
	Plane _far;
	Plane _near;
};
