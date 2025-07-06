#pragma once
#include <array>
#include "AABB.hpp"
#include "Matrix4.hpp"
#include "Plane.hpp"
#include "Vector4.hpp"


class CameraComponent;
class TransformComponent;

class Frustrum
{
public:
	Frustrum();
	Frustrum(const CameraComponent &camera);
	Frustrum(const Plane& left, const Plane& right, const Plane& top, 
	         const Plane& bottom, const Plane& nearPlane, const Plane& farPlane);

	bool contains(const Aabb &box, const TransformComponent &transform) const;

private:
	// Helper methods for optimized culling
	bool containsAxisAligned(const Aabb &aabb, const TransformComponent &transform) const;
	bool containsOriented(const Aabb &aabb, const TransformComponent &transform) const;
	bool isTransformAxisAligned(const TransformComponent &transform) const;

	Plane _left;
	Plane _right;
	Plane _top;
	Plane _bottom;
	Plane _far;
	Plane _near;
};
