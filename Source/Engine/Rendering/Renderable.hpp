#pragma once
#include <memory>

#include "Renderer.h"
#include "../Maths/Matrix4.hpp"
#include "../SceneManagement/Component.hpp"

class GpuBuffer;
class StaticMesh;

struct PerObjectBufferData
{
	Matrix4 Model;
	Matrix4 ModelView;
	Matrix4 ModelViewProjection;
};

class Renderable final : public Component
{
public:
	Renderable();

	void DrawInspector() override;
	void Draw(const std::shared_ptr<Renderer>& renderer) override;
	void Update() override;

	void SetMesh(const std::shared_ptr<StaticMesh>& mesh);

	std::shared_ptr<StaticMesh> GetMesh() const;
	std::shared_ptr<GpuBuffer> GetPerObjectBuffer() const;

	void DrawBoundingBox(bool draw) { _drawBoundingBox = draw; }
	bool ShouldDrawBoundingBox() const { return _drawBoundingBox; }

	Aabb GetAabb() const { return _currAabb; }

private:
	void UpdateAabb(Vector3 scalingDelta, Quaternion rotationDelta);

	Aabb _initAabb, _currAabb;
	bool _drawBoundingBox;

	Vector3 _currentRotationEuler;
	Vector3 _currentScale;
	
  std::shared_ptr<StaticMesh> _mesh;
	std::shared_ptr<GpuBuffer> _perObjectBuffer;
};
