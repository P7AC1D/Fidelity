#pragma once
#include "SceneNode.hpp"
#include "../Maths/Frustrum.hpp"

class CameraNode final : public SceneNode
{
  friend class SceneNode;
	
public:  
	CameraNode(const CameraNode& other) = default;
	CameraNode(CameraNode&& other) noexcept = default;
	CameraNode& operator=(const CameraNode& other) = default;
	CameraNode& operator=(CameraNode&& other) noexcept = default;	
  ~CameraNode() = default;

  void OnDrawInspector() override;
  void OnDraw(std::shared_ptr<Renderer> renderer) override;
  void OnUpdate(float64 dt) override;

  void SetPerspective(const Degree& fovY, int32 width, int32 height, float32 nearClip, float32 farClip);
	
  Matrix4 GetView() const;
  Matrix4 GetProj() const;

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

  SceneNodeType GetNodeType() const override;

protected:
  CameraNode();
	
private:
  void UpdateView();
  void UpdateProjection();
	
  int32 _width;
  int32 _height;
  Radian _fov;
  float32 _near;
  float32 _far;

  Matrix4 _view;
  Matrix4 _proj;
};