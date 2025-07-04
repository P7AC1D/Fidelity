#include "Camera.h"

#include "../UI/ImGui/imgui.h"
#include "Drawable.h"

Camera::Camera() : _width(1280),
									 _height(768),
									 _fov(Degree(60.f)),
									 _near(0.1f),
									 _far(10000.0f),
									 _view(Matrix4::Identity),
									 _proj(Matrix4::Identity),
									 _modified(true),
									 _fixFrustrum(false),
									 _frustumDirty(true),
									 Component(ComponentType::Camera)
{
	updateProjection();
}

void Camera::drawInspector()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		float32 farClip = _far;
		float32 nearClip = _near;
		float32 fovY = _fov.InDegrees();

		if (ImGui::SliderFloat("Near Clip", &nearClip, 0.1f, 100.f))
		{
			setNear(nearClip);
		}

		if (ImGui::SliderFloat("Far Clip", &farClip, 0.1f, 10000.f))
		{
			setFar(farClip);
		}

		if (ImGui::SliderFloat("Field-of-view", &fovY, 1.f, 180.f))
		{
			setFov(fovY);
		}

		bool fixFrustrum = _fixFrustrum;
		if (ImGui::Checkbox("Fix Frustrum", &fixFrustrum))
		{
			_fixFrustrum = fixFrustrum;
			_fixedFrustrum = _frustrum;
		}
	}
}

Camera &Camera::setPerspective(const Degree &fovY, int32 width, int32 height, float32 nearClip, float32 farClip)
{
	setWidth(width);
	setHeight(height);
	setFov(fovY);
	setNear(nearClip);
	setFar(farClip);
	_modified = true;
	return *this;
}

Camera &Camera::setHeight(int32 height)
{
	_height = height;
	_modified = true;
	invalidateFrustumCache();
	return *this;
}

Camera &Camera::setWidth(int32 width)
{
	_width = width;
	_modified = true;
	invalidateFrustumCache();
	return *this;
}

Camera &Camera::setFov(const Degree &fov)
{
	_fov = Radian(fov);
	_modified = true;
	invalidateFrustumCache();
	return *this;
}

Camera &Camera::setNear(float32 near)
{
	_near = near;
	_modified = true;
	invalidateFrustumCache();
	return *this;
}

Camera &Camera::setFar(float32 far)
{
	_far = far;
	_modified = true;
	invalidateFrustumCache();
	return *this;
}

void Camera::onUpdate(float32 dt)
{
	if (_modified)
	{
		updateProjection();
		_modified = false;
	}
}

void Camera::onNotify(const GameObject &gameObject)
{
	Transform transform(gameObject.getLocalTransform());
	updateView(transform);
	_transform = transform;
}

void Camera::updateView(const Transform &transform)
{
	Matrix4 rotation(transform.getRotation());
	Matrix4 translation(Matrix4::Translation(transform.getPosition()));
	translation[3][0] = -translation[3][0];
	translation[3][1] = -translation[3][1];
	translation[3][2] = -translation[3][2];

	_view = rotation * translation;
	invalidateFrustumCache();
}

void Camera::updateProjection()
{
	_proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
	invalidateFrustumCache();
}

const Frustrum &Camera::getFrustumCached() const
{
	if (_frustumDirty)
	{
		// Only create frustum if we have a valid transform
		// This prevents creating frustum with uninitialized transform data
		if (_transform.getPosition() != Vector3::Zero || _transform.getRotation() != Quaternion::Identity)
		{
			_frustrum = Frustrum(*this);
		}
		_frustumDirty = false;
	}
	return _frustrum;
}

void Camera::invalidateFrustumCache()
{
	_frustumDirty = true;
}

bool Camera::contains(const Aabb &aabb, const Transform &transform) const
{
	if (_fixFrustrum)
	{
		return _fixedFrustrum.contains(aabb, transform);
	}

	// Check if we have a valid camera transform before doing frustum culling
	// If transform is not initialized (all zeros), assume everything is visible
	if (_transform.getPosition() == Vector3::Zero && _transform.getRotation() == Quaternion::Identity)
	{
		// Camera transform not yet initialized, don't cull anything
		return true;
	}

	return getFrustumCached().contains(aabb, transform);
}

float32 Camera::distanceFrom(const Vector3 &position) const
{
	return (_transform.getPosition() - position).Length();
}
