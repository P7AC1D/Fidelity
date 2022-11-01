#include "CameraBuilder.hpp"

#include "Camera.h"

CameraBuilder::CameraBuilder() : _width(1280),
                                 _height(768),
                                 _fovY(Degree(60.f)),
                                 _nearClip(0.1f),
                                 _farClip(100.0f)
{
}

CameraBuilder &CameraBuilder::withHeight(int32 height)
{
  _height = height;
  return *this;
}

CameraBuilder &CameraBuilder::withWidth(int32 width)
{
  _width = width;
  return *this;
}

CameraBuilder &CameraBuilder::withFov(const Degree &fov)
{
  _fovY = fov;
  return *this;
}

CameraBuilder &CameraBuilder::withNear(float32 near)
{
  _nearClip = near;
  return *this;
}

CameraBuilder &CameraBuilder::withFar(float32 far)
{
  _farClip = far;
  return *this;
}

std::shared_ptr<Component> CameraBuilder::build()
{
  std::shared_ptr<Camera> camera(new Camera());
  camera->setFar(_farClip);
  camera->setNear(_nearClip);
  camera->setFov(_fovY);
  camera->setWidth(_width);
  camera->setHeight(_height);
  return camera;
}