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
	return *this;
}

Camera &Camera::setWidth(int32 width)
{
	_width = width;
	_modified = true;
	return *this;
}

Camera &Camera::setFov(const Degree &fov)
{
	_fov = Radian(fov);
	_modified = true;
	return *this;
}

Camera &Camera::setNear(float32 near)
{
	_near = near;
	_modified = true;
	return *this;
}

Camera &Camera::setFar(float32 far)
{
	_far = far;
	_modified = true;
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
	_frustrum = Frustrum(*this);
}

void Camera::updateProjection()
{
	_proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
	_frustrum = Frustrum(*this);
}

bool Camera::contains(const Aabb &aabb, const Transform &transform) const
{
	if (_fixFrustrum)
	{
		return _fixedFrustrum.contains(aabb, transform);
	}

	return _frustrum.contains(aabb, transform);
}

float32 Camera::distanceFrom(const Vector3 &position) const
{
	return (_transform.getPosition() - position).Length();
}