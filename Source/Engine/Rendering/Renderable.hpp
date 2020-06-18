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
	friend class SceneManager;

public:
	Renderable();

	void DrawInspector() override;
	void Draw(const std::shared_ptr<Renderer>& renderer) override;
	void Update() override;

	void SetMesh(const std::shared_ptr<StaticMesh>& mesh);

	std::shared_ptr<StaticMesh> GetMesh() const;
	std::shared_ptr<GpuBuffer> GetPerObjectBuffer() const;

private:
  std::shared_ptr<StaticMesh> _mesh;
	std::shared_ptr<GpuBuffer> _perObjectBuffer;
};
