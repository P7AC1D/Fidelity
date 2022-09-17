#include "Frustrum.hpp"

#include "../Rendering/Camera.h"
#include "Math.hpp"

Frustrum::Frustrum()
{
}

Frustrum::Frustrum(const Camera &camera)
{
	const Transform &cameraTransform(camera.getParentTransform());
	const float32 far = camera.getFar();
	const Vector3 position = cameraTransform.getPosition();
	const Vector3 forward = cameraTransform.getForward();
	const Vector3 up = cameraTransform.getUp();
	const Vector3 right = cameraTransform.getRight();

	const float32 halfVSide = far * tanf(camera.getFov().InRadians() * .5f);
	const float32 halfHSide = halfVSide * camera.getAspectRatio();

	const Vector3 frontMultFar = far * forward;
	_near = std::move(Plane(position + camera.getNear() * forward, forward));
	_far = std::move(Plane(position + frontMultFar, -forward));
	_right = std::move(Plane(position, Vector3::Cross(cameraTransform.getUp(), frontMultFar + right * halfHSide)));
	_left = std::move(Plane(position, Vector3::Cross(frontMultFar - right * halfHSide, up)));
	_top = std::move(Plane(position, Vector3::Cross(right, frontMultFar - up * halfVSide)));
	_bottom = std::move(Plane(position, Vector3::Cross(frontMultFar + up * halfVSide, right)));
}

bool Frustrum::contains(const Aabb &aabb) const
{
	return aabb.isInFrustrumPlane(_near) &&
				 aabb.isInFrustrumPlane(_far) &&
				 aabb.isInFrustrumPlane(_left) &&
				 aabb.isInFrustrumPlane(_right) &&
				 aabb.isInFrustrumPlane(_top) &&
				 aabb.isInFrustrumPlane(_bottom);
}