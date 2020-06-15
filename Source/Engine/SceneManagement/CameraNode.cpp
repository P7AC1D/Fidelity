#include "CameraNode.hpp"

#include "../Maths/Degree.hpp"
#include "../Rendering/Renderer.h"

CameraNode::CameraNode():
	_width(1280),
_height(768),
_fov(Degree(60.f)),
_near(0.1f),
_far(10000.0f)
{
	
}

void CameraNode::OnDraw(std::shared_ptr<Renderer> renderer)
{
	renderer->BindCamera(std::dynamic_pointer_cast<CameraNode>(shared_from_this()));
}

void CameraNode::OnUpdate(float64 dt)
{
	UpdateView();
	UpdateProjection();
}

Matrix4 CameraNode::GetView() const
{
	return _view;
}

Matrix4 CameraNode::GetProj() const
{
	return _proj;
}

int32 CameraNode::GetWidth() const
{
	return _width;
}

void CameraNode::SetWidth(int32 width)
{
	_width = width;
}

int32 CameraNode::GetHeight() const
{
	return _height;
}

void CameraNode::SetHeight(int32 height)
{
	_height = height;
}

Radian CameraNode::GetFov() const
{
	return _fov;
}

void CameraNode::SetFov(const Degree& fov)
{
	_fov = Radian(fov);
}

float32 CameraNode::GetNear() const
{
	return _near;
}

void CameraNode::SetNear(float32 near)
{
	_near = near;
}

float32 CameraNode::GetFar() const
{
	return _far;
}

void CameraNode::SetFar(float32 far)
{
	_far = far;
}

void CameraNode::UpdateView()
{
	Transform& transform = GetTransform();
	Matrix4 rotation(transform.GetRotation());
	Matrix4 translation(Matrix4::Translation(transform.GetPosition()));
	translation[3][0] = -translation[3][0];
	translation[3][1] = -translation[3][1];
	translation[3][2] = -translation[3][2];

	_view = rotation * translation;
}

void CameraNode::UpdateProjection()
{
	_proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
}