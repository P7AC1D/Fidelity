#include "CameraNode.hpp"

#include "../Maths/Degree.hpp"

CameraNode::CameraNode():
	_width(1280),
_height(768),
_fov(Degree(60.f)),
_near(0.1f),
_far(10000.0f),
_projection(CameraProjection::Projection)
{
	
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

CameraProjection CameraNode::GetProjection() const
{
	return _projection;
}

void CameraNode::SetProjection(CameraProjection projection)
{
	_projection = projection;
}
