#include "Camera.h"

#include "../UI/ImGui/imgui.h"
#include "Drawable.h"

Camera::Camera() : _width(1280),
									 _height(768),
									 _fov(Degree(60.f)),
									 _near(0.1f),
									 _far(10000.0f),
									 _modified(true)
{
	updateProjection();
}

void Camera::drawInspector()
{
	ImGui::Separator();
	ImGui::Text("Camera");
	{
		float32 farClip = _far;
		float32 nearClip = _near;
		float32 fovY = _fov.InDegrees();

		ImGui::DragFloat("Near Clip", &nearClip, 1, 0.1f, 100.f);
		ImGui::DragFloat("Far Clip", &farClip, 1, 0.1f, 10000.f);
		ImGui::DragFloat("Field-of-view", &fovY, 1, 1.f, 180.f);

		setFar(farClip);
		setNear(nearClip);
		setFov(fovY);

		Vector3 position(_transform.getPosition());
		float32 pos[]{position.X, position.Y, position.Z};
		ImGui::DragFloat3("Position", pos, 0.1f);
		_transform.setPosition(Vector3(pos[0], pos[1], pos[2]));

		Vector3 euler = _transform.getRotation().ToEuler();
		float32 angles[3] = {euler.X, euler.Y, euler.Z};
		if (ImGui::DragFloat3("Orientation", angles, 1.0f))
		{
			_transform.rotate(Quaternion(Degree(euler.X - angles[0]), Degree(euler.Y - angles[1]), Degree(euler.Z - angles[2])));
		}
	}
}

void Camera::update(float64 dt)
{
	if (_modified || _transform.modified())
	{
		_transform.update(dt);
		updateView();
		updateProjection();
		_modified = false;
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

void Camera::updateView()
{
	Matrix4 rotation(_transform.getRotation());
	Matrix4 translation(Matrix4::Translation(_transform.getPosition()));
	translation[3][0] = -translation[3][0];
	translation[3][1] = -translation[3][1];
	translation[3][2] = -translation[3][2];

	_view = rotation * translation;
}

void Camera::updateProjection()
{
	_proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
	_frustrum = Frustrum(_proj);
}

bool Camera::intersectsFrustrum(const Aabb &aabb) const
{
	return _frustrum.Intersects(aabb);
}

float32 Camera::distanceFrom(const Vector3 &position) const
{
	return (_transform.getPosition() - position).Length();
}