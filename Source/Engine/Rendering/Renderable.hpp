#pragma once
#include <memory>
#include <vector>
#include "../SceneManagement/Component.hpp"

class StaticMesh;

class Renderable : public Component
{
public:
	void Update() override;

	void SetMesh(const std::shared_ptr<StaticMesh>& mesh);

	std::shared_ptr<StaticMesh> GetMesh() const;

private:
  std::shared_ptr<StaticMesh> _mesh;  
};
