#pragma once
#include <memory>
#include <vector>
#include "../Maths/Matrix4.hpp"
#include "../SceneManagement/Component.hpp"

class GpuBuffer;
class StaticMesh;

struct PerObjectBufferData
{
	Matrix4 Model;
};

class Renderable : public Component
{
public:
	Renderable();

	void Update() override;
	void UpdatePerObjectBuffer(const PerObjectBufferData& perObjectData);

	void SetMesh(const std::shared_ptr<StaticMesh>& mesh);

	std::shared_ptr<StaticMesh> GetMesh() const;
	std::shared_ptr<GpuBuffer> GetPerObjectBuffer() const;

private:
  std::shared_ptr<StaticMesh> _mesh;
	std::shared_ptr<GpuBuffer> _perObjectBuffer;
};
