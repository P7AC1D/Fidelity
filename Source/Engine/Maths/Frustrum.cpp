#include "Frustrum.hpp"

#include "../Core/Transform.h"
#include "../Rendering/Camera.h"
#include "Math.hpp"

Frustrum::Frustrum()
{
}

Frustrum::Frustrum(const Camera &camera)
{
	// Taken from https://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-extracting-the-planes/

	const Transform &cameraTransform(camera.getParentTransform());
	Vector3 position = cameraTransform.getPosition();
	Vector3 forward = cameraTransform.getForward();
	Vector3 up = cameraTransform.getUp();
	Vector3 right = cameraTransform.getRight();
	float32 zFar = camera.getFar();
	float32 zNear = camera.getNear();
	float32 tanHalfFov = (float32)tan(camera.getFov().InRadians() / 2.0f);
	float32 farHeight = zFar * tanHalfFov;
	float32 farWidth = farHeight * camera.getAspectRatio();

	// Calculate the center of the far plane by taking the position and scaling the forward vector by the distance to the far plane.
	// NOTE: Using subtraction (-) because the camera's forward vector points in the negative Z direction in OpenGL convention
	Vector3 zFarCenter = position - forward * zFar;
	// Similarly, we calculate the center of the near plane.
	Vector3 zNearCenter = position - forward * zNear;

	// Build planes with inwards facing normals and the position of the camera as it is present in all top, bottom, left and right planes.
	_near = Plane(-forward, zNearCenter);
	_far = Plane(forward, zFarCenter);

	Vector3 point = zFarCenter + (up * farHeight);
	Vector3 normal = Vector3::Normalize(point - position);
	normal = Vector3::Cross(normal, right);
	_top = Plane(normal, position);

	point = zFarCenter - (up * farHeight);
	normal = Vector3::Normalize(point - position);
	normal = Vector3::Cross(right, normal);
	_bottom = Plane(normal, position);

	point = zFarCenter - (right * farWidth);
	normal = Vector3::Normalize(point - position);
	normal = Vector3::Cross(normal, up);
	_left = Plane(normal, position);

	point = zFarCenter + (right * farWidth);
	normal = Vector3::Normalize(point - position);
	normal = Vector3::Cross(up, normal);
	_right = Plane(normal, position);
}

bool Frustrum::contains(const Aabb &aabb, const Transform &transform) const
{
	// Check if transform is axis-aligned (identity rotation) for optimization
	if (isTransformAxisAligned(transform))
	{
		return containsAxisAligned(aabb, transform);
	}
	
	// Handle oriented bounding boxes with full transform
	return containsOriented(aabb, transform);
}

bool Frustrum::isTransformAxisAligned(const Transform &transform) const
{
	return transform.isAxisAligned();
}

bool Frustrum::containsAxisAligned(const Aabb &aabb, const Transform &transform) const
{
	// Simple case: just translate the AABB center and test directly
	Vector3 globalCenter = transform.getPosition() + aabb.getCenter();
	Aabb globalAabb(globalCenter, aabb.getExtents().X, aabb.getExtents().Y, aabb.getExtents().Z);
	
	return globalAabb.isOnOrForwardPlane(_near) &&
				 globalAabb.isOnOrForwardPlane(_far) &&
				 globalAabb.isOnOrForwardPlane(_right) &&
				 globalAabb.isOnOrForwardPlane(_left) &&
				 globalAabb.isOnOrForwardPlane(_top) &&
				 globalAabb.isOnOrForwardPlane(_bottom);
}

bool Frustrum::containsOriented(const Aabb &aabb, const Transform &transform) const
{
	Vector3 extents(aabb.getExtents());
	Vector3 globalCenter(transform.getPosition() + aabb.getCenter());

	// Transform the AABB axes by the object's rotation
	Vector3 right(transform.getRight() * extents.X);
	Vector3 up(transform.getUp() * extents.Y);
	Vector3 forward(transform.getForward() * extents.Z);

	// Calculate the global extents by projecting the transformed axes onto world axes
	// This gives us the axis-aligned bounding box that contains the oriented box
	Vector3 globalExtents(
		std::abs(right.X) + std::abs(up.X) + std::abs(forward.X),
		std::abs(right.Y) + std::abs(up.Y) + std::abs(forward.Y),
		std::abs(right.Z) + std::abs(up.Z) + std::abs(forward.Z)
	);

	Aabb globalAabb(globalCenter, globalExtents.X, globalExtents.Y, globalExtents.Z);

	return globalAabb.isOnOrForwardPlane(_near) &&
				 globalAabb.isOnOrForwardPlane(_far) &&
				 globalAabb.isOnOrForwardPlane(_right) &&
				 globalAabb.isOnOrForwardPlane(_left) &&
				 globalAabb.isOnOrForwardPlane(_top) &&
				 globalAabb.isOnOrForwardPlane(_bottom);
}
