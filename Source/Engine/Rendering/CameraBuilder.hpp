#pragma once

#include "../Core/Component.h"
#include "../Core/ComponentBuilder.hpp"
#include "../Core/Types.hpp"
#include "../Core/Maths.h"

class CameraBuilder : public ComponentBuilder
{
public:
  CameraBuilder();

  CameraBuilder &withHeight(int32 height);
  CameraBuilder &withWidth(int32 width);
  CameraBuilder &withFov(const Degree &fov);
  CameraBuilder &withNear(float32 near);
  CameraBuilder &withFar(float32 far);

  std::shared_ptr<Component> build() override;

private:
  Degree _fovY;
  int32 _width, _height;
  float32 _nearClip, _farClip;
};