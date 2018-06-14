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
  Light& CreateLight(LightType lightType, const std::string& name = std::string());
	
  inline void SetAmbientLight(const Colour& colour) { _ambientLight = colour; }

	void SetCamera(std::shared_ptr<Camera> camera);

  inline const Colour& GetAmbientLight() const { return _ambientLight; }

  void UpdateScene(uint32 dtMs);

private:
  void UpdateWorldObjects(uint32 dtMs);
  void SubmitSceneToRender();

private:
  std::vector<std::shared_ptr<WorldObject>> _worldObjects;
  std::vector<Light> _lights;
  std::shared_ptr<Renderer> _renderer;
  std::shared_ptr<Camera> _camera;
  Colour _ambientLight;
};
