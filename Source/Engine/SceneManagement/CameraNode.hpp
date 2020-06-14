#pragma once
#include "SceneNode.hpp"
#include "../Maths/Frustrum.hpp"

enum class CameraProjection
{
	Orthographic,
	Projection
};

class CameraNode final : SceneNode
{
public:
  CameraNode();
	CameraNode(const CameraNode& other) = default;
	CameraNode(CameraNode&& other) noexcept = default;
	CameraNode& operator=(const CameraNode& other) = default;
	CameraNode& operator=(CameraNode&& other) noexcept = default;	
  ~CameraNode() = default;

  void OnUpdate(float64 dt) override {}

  int32 GetWidth() const;
  void SetWidth(int32 width);
  int32 GetHeight() const;
  void SetHeight(int32 height);
  Radian GetFov() const;
  void SetFov(const Degree& fov);
  float32 GetNear() const;
  void SetNear(float32 near);
  float32 GetFar() const;
  void SetFar(float32 far);
  CameraProjection GetProjection() const;
  void SetProjection(CameraProjection projection);
	
private:
  int32 _width;
  int32 _height;
  Radian _fov;
  float32 _near;
  float32 _far;
  CameraProjection _projection;
};