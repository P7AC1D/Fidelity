#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "Light.h"

class Actor;
class Camera;
class Ray;
class Renderer;
class SceneNode;

class SceneManager
{
	friend class Actor;

public:
	static std::shared_ptr<SceneManager> Get();

  void SetRenderer(const std::shared_ptr<Renderer>& renderer);

  std::shared_ptr<Light> CreateLight(LightType lightType, const std::string& name = std::string());
  std::shared_ptr<SceneNode> LoadModelFromFile(const std::string& filePath, bool reconstructWorldTransforms = false);

	void SetCamera(const std::shared_ptr<Camera>& camera);
	void SetDirectionLight(const std::shared_ptr<Light>& light);
  void SetAmbientLightColour(const Colour& colour);
  void SetAmbientLightIntensity(float32 intensity);

	std::shared_ptr<SceneNode> GetRootSceneNode() const;
  std::shared_ptr<Camera> GetCamera() const;
	std::shared_ptr<Light> GetDirectionalLight() const;
  Colour GetAmbientLightColour() const;
  float32 GetAmbientLightIntensity() const;
  
  void UpdateScene(uint32 dtMs);

private:
	SceneManager();

	void PushRenderable(Actor* actor);
  void SubmitSceneToRender();

private:
	static std::shared_ptr<SceneManager> Instance;
	std::shared_ptr<SceneNode> _sceneGraph;
  std::vector<std::shared_ptr<Light>> _lights;
	std::vector<Actor*> _renderableActors;
	std::shared_ptr<Light> _directionalLight;
  std::shared_ptr<Renderer> _renderer;
  std::shared_ptr<Camera> _camera;
  Colour _ambientLightColour;
  float32 _ambientLightIntensity;
};
