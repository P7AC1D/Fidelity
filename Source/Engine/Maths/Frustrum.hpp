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
	bool contains(const Aabb &box, const Matrix4 &transform) const;

private:
	// Helper methods for optimized culling
	bool containsAxisAligned(const Aabb &aabb, const TransformComponent &transform) const;
	bool containsOriented(const Aabb &aabb, const TransformComponent &transform) const;
	bool isTransformAxisAligned(const TransformComponent &transform) const;
	
	// Matrix4 optimization helpers
	bool isMatrix4AxisAligned(const Matrix4 &transform) const;
	bool containsAxisAlignedMatrix4(const Aabb &aabb, const Matrix4 &transform) const;
	bool containsOrientedMatrix4(const Aabb &box, const Matrix4 &transform) const;
	
	// Frustum plane extraction and testing
	void extractPlanesFromMatrix(const Matrix4& viewProjMatrix);
	bool testAABBAgainstPlane(const Vector3& min, const Vector3& max, const Plane& plane) const;
	Plane createPlaneFromVector4(const Vector4& planeVector) const;

	Plane _left;
	Plane _right;
	Plane _top;
	Plane _bottom;
	Plane _far;
	Plane _near;
};
