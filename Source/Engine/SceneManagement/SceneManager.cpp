#include "SceneManager.h"

#include <chrono>
#include "../Components/Component.h"
#include "../Rendering/Renderer.h"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/WorldObject.h"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "Camera.hpp"
#include "WorldObject.h"

static uint32 WorldObjectCount = 0;
static uint32 LightCount = 0;

SceneManager::SceneManager(const std::shared_ptr<Renderer>& renderer) :
  _renderer(renderer),
  _ambientLightColour(Colour::White),
  _ambientLightIntensity(0.1f)
{
}

std::shared_ptr<WorldObject> SceneManager::CreateObject(const std::string& name)
{
  std::string objectName(name); 
  if (name == std::string())
  {
    objectName = "WorldObject" + std::to_string(WorldObjectCount);
    WorldObjectCount++;
  }
  _worldObjects.emplace_back(std::make_shared<WorldObject>(objectName));
  return _worldObjects.back();
}

std::shared_ptr<Light> SceneManager::CreateLight(LightType lightType, const std::string& name)
{
  std::string objectName(name);
  if (name == std::string())
  {
    objectName = "Light" + std::to_string(LightCount);
    LightCount++;
  }

	std::shared_ptr<Light> light(new Light(lightType, name));
	_lights.push_back(light);
	return light;
}

void SceneManager::SetCamera(const std::shared_ptr<Camera>& camera)
{
	_camera = camera;
	_renderer->SetCamera(camera);
}

void SceneManager::SetDirectionLight(const std::shared_ptr<Light>& light)
{
	Assert::ThrowIfFalse(light->GetType() == LightType::Directional, "Light must be directional");
	_renderer->SetDirectionalLight(DirectionalLightData(light->GetColour(), light->GetDirection(), light->GetIntensity()));
	_directionalLight = light;
}

void SceneManager::SetAmbientLightColour(const Colour& colour)
{
  _renderer->SetAmbientLight({ colour, _ambientLightIntensity });
  _ambientLightColour = colour;
}

void SceneManager::SetAmbientLightIntensity(float32 intensity)
{  
  _ambientLightIntensity  = intensity;
}

Colour SceneManager::GetAmbientLightColour() const
{
  return _ambientLightColour;
}

float32 SceneManager::GetAmbientLightIntensity() const
{
  return _ambientLightIntensity;
}

std::shared_ptr<Camera> SceneManager::GetCamera() const
{
  return _camera;
}

std::shared_ptr<Light> SceneManager::GetDirectionalLight() const
{
	return _directionalLight;
}

void SceneManager::UpdateScene(uint32 dtMs)
{
  UpdateWorldObjects(dtMs);
  SubmitSceneToRender();
}

void SceneManager::UpdateWorldObjects(uint32 dtMs)
{
  for (auto worldObject : _worldObjects)
  {
    worldObject->Update();
  }
}

void SceneManager::SubmitSceneToRender()
{
  _renderer->SetAmbientLight(AmbientLightData(_ambientLightColour, _ambientLightIntensity));
	_renderer->SetDirectionalLight(DirectionalLightData(_directionalLight->GetColour(), _directionalLight->GetDirection(), _directionalLight->GetIntensity()));
  for (auto worldObject : _worldObjects)
  {
    auto renderable = worldObject->GetRenderable();
    if (renderable != nullptr)
    {
      auto transform = worldObject->GetTransform();
      _renderer->PushRenderable(renderable, transform);
    }

    auto lightComponent = worldObject->GetComponent("Light");
    if (lightComponent != nullptr)
    {
      auto light = std::dynamic_pointer_cast<Light>(lightComponent);
    }
  }
}
