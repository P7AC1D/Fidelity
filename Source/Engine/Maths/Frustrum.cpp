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
	// Negated here as the camera's forward vector actually points in the oposite direction
	Vector3 forward = -Vector3::Normalize(cameraTransform.getForward());
	Vector3 up = Vector3::Normalize(cameraTransform.getUp());
	Vector3 right = Vector3::Normalize(cameraTransform.getRight());
	float32 zFar = camera.getFar();
	float32 zNear = camera.getNear();
	float32 nearHeight = 2.0f * tanf(camera.getFov().InRadians() * 0.5f) * zNear;
	float32 nearWidth = nearHeight * camera.getAspectRatio();
	float32 farHeight = 2.0f * tanf(camera.getFov().InRadians() * 0.5f) * zFar;
	float32 farWidth = farHeight * camera.getAspectRatio();

	// Calculate the center of the far plane by taking the position and scaling the forward vector by the distance to the far plane.
	Vector3 zFarCenter = position + forward * zFar;
	// Similarly, we calculate the center of the near plane.
	Vector3 zNearCenter = position + forward * zNear;

	// Build planes with inwards facing normals and the position of the camera as it is present in all top, bottom, left and right planes.
	_near = Plane(forward, zNearCenter);
	_far = Plane(-forward, zFarCenter);
	_right = Plane(Vector3::Cross(up, zNearCenter + (right * nearWidth * 0.5f) - position), position);
	_left = Plane(Vector3::Cross(zNearCenter - (right * nearWidth * 0.5f) - position, up), position);
	_bottom = Plane(Vector3::Cross(right, zNearCenter - (up * nearHeight * 0.5f) - position), position);
	_top = Plane(Vector3::Cross(zNearCenter + (up * nearHeight * 0.5f) - position, right), position);
}

bool Frustrum::contains(const Aabb &aabb, const Transform &transform) const
{
	Vector3 extents(aabb.getExtents());
	Vector3 globalCenter(Matrix4::Translation(transform.getPosition()) * Matrix4::Scaling(aabb.getExtents()) * aabb.getCenter());

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