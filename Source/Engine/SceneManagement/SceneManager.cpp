#include "SceneManager.h"

#include <chrono>
#include "../Rendering/Renderable.hpp"
#include "../Rendering/Renderer.h"
#include "../Utility/Assert.hpp"
#include "../Utility/ModelLoader.hpp"
#include "../Utility/String.hpp"
#include "Actor.hpp"
#include "Camera.hpp"
#include "SceneNode.hpp"
#include "Transform.h"

static uint32 LightCount = 0;

SceneManager::SceneManager(const std::shared_ptr<Renderer>& renderer) :
  _sceneGraph(new SceneNode("root")),
  _renderer(renderer),
  _ambientLightColour(Colour::White),
  _ambientLightIntensity(0.1f)
{
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

std::shared_ptr<SceneNode> SceneManager::LoadModelFromFile(const std::string& filePath)
{
  std::shared_ptr<SceneNode> modelNode = ModelLoader::LoadFromFile(filePath);
  _sceneGraph->AddChildNode(modelNode);
  return modelNode;
}

void SceneManager::SetCamera(const std::shared_ptr<Camera>& camera)
{
	_camera = camera;
	_renderer->SetCamera(camera);
}

void SceneManager::SetDirectionLight(const std::shared_ptr<Light>& light)
{
	ASSERT_TRUE(light->GetType() == LightType::Directional, "Light must be directional");
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

std::shared_ptr<SceneNode> SceneManager::GetRootSceneNode() const
{
	return _sceneGraph;
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
  SubmitSceneToRender();
}

void SceneManager::SubmitSceneToRender()
{
  _renderer->SetAmbientLight(AmbientLightData(_ambientLightColour, _ambientLightIntensity));
	_renderer->SetDirectionalLight(DirectionalLightData(_directionalLight->GetColour(), _directionalLight->GetDirection(), _directionalLight->GetIntensity()));
  
	auto actors = _sceneGraph->GetAllActors();

	std::vector<std::shared_ptr<Actor>> zSortedActors;
	zSortedActors.reserve(actors.size());

	for (auto actor : actors)
	{
		auto renderable = actor->GetComponent<Renderable>();		
		if (renderable)
		{
			auto transform = actor->GetTransform();
			zSortedActors.push_back(actor);
		}
	}

	std::sort(zSortedActors.begin(), zSortedActors.end(), [&](const std::shared_ptr<Actor>& a, const std::shared_ptr<Actor>& b)
	{
		auto camPos = _camera->GetPosition();
		
		auto posA = a->GetTransform()->GetPosition() + a->GetBounds().GetMidPoint();
		auto posB = b->GetTransform()->GetPosition() + b->GetBounds().GetMidPoint();

		auto zDistanceA = Vector3::Length(camPos - posA);
		auto zDistanceB = Vector3::Length(camPos - posB);

		return zDistanceA < zDistanceB;
	});

	for (auto actor : zSortedActors)
	{
		_renderer->PushRenderable(actor->GetComponent<Renderable>(), actor->GetTransform());
	}
}
