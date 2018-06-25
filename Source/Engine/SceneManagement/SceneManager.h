#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "Light.h"

class Camera;
class Renderer;
class WorldObject;

class SceneManager
{
public:
  SceneManager(const std::shared_ptr<Renderer>& renderer);

  std::shared_ptr<WorldObject> CreateObject(const std::string& name = std::string());  
  std::shared_ptr<Light> CreateLight(LightType lightType, const std::string& name = std::string());

	void SetCamera(const std::shared_ptr<Camera>& camera);
	void SetDirectionLight(const std::shared_ptr<Light>& light);

  std::shared_ptr<Camera> GetCamera() const;

  void UpdateScene(uint32 dtMs);

private:
  void UpdateWorldObjects(uint32 dtMs);
  void SubmitSceneToRender();

private:
  std::vector<std::shared_ptr<WorldObject>> _worldObjects;
  std::vector<std::shared_ptr<Light>> _lights;
	std::shared_ptr<Light> _directionalLight;
  std::shared_ptr<Renderer> _renderer;
  std::shared_ptr<Camera> _camera;
};
