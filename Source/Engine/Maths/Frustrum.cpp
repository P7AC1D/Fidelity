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
	Vector3 extents(aabb.getExtents());
	Vector3 globalCenter(transform.getPosition() + aabb.getCenter());

	Vector3 right(transform.getRight() * extents.X);
	Vector3 up(transform.getUp() * extents.Y);
	Vector3 forward(transform.getForward() * extents.Z);

	Vector3 globalExtents(std::abs(Vector3::Dot(Vector3{1.f, 0.f, 0.f}, right)) +
														std::abs(Vector3::Dot(Vector3{1.f, 0.f, 0.f}, up)) +
														std::abs(Vector3::Dot(Vector3{1.f, 0.f, 0.f}, forward)),
												std::abs(Vector3::Dot(Vector3{0.f, 1.f, 0.f}, right)) +
														std::abs(Vector3::Dot(Vector3{0.f, 1.f, 0.f}, up)) +
														std::abs(Vector3::Dot(Vector3{0.f, 1.f, 0.f}, forward)),
												std::abs(Vector3::Dot(Vector3{0.f, 0.f, 1.f}, right)) +
														std::abs(Vector3::Dot(Vector3{0.f, 0.f, 1.f}, up)) +
														std::abs(Vector3::Dot(Vector3{0.f, 0.f, 1.f}, forward)));

	Aabb globalAabb(globalCenter, globalExtents.X, globalExtents.Y, globalExtents.Z);

	return globalAabb.isOnOrForwardPlane(_near) &&
				 globalAabb.isOnOrForwardPlane(_far) &&
				 globalAabb.isOnOrForwardPlane(_right) &&
				 globalAabb.isOnOrForwardPlane(_left) &&
				 globalAabb.isOnOrForwardPlane(_top) &&
				 globalAabb.isOnOrForwardPlane(_bottom);
}