#include "SceneManager.h"

#include <chrono>
#include "../Maths/Frustrum.hpp"
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
std::shared_ptr<SceneManager> SceneManager::Instance;

std::shared_ptr<SceneManager> SceneManager::Get()
{
	if (!Instance)
	{
		Instance.reset(new SceneManager);
	}
	return Instance;
}

void SceneManager::SetRenderer(const std::shared_ptr<Renderer>& renderer)
{
	_renderer = renderer;
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

std::shared_ptr<SceneNode> SceneManager::LoadModelFromFile(const std::string& filePath, bool reconstructWorldTransforms)
{
  std::shared_ptr<SceneNode> modelNode = ModelLoader::LoadFromFile(filePath, reconstructWorldTransforms);
  _sceneGraph->AddChildNode(modelNode);
  return modelNode;
}

void SceneManager::SetCamera(const std::shared_ptr<Camera>& camera)
{
	_camera = camera;
}

void SceneManager::SetDirectionLight(const std::shared_ptr<Light>& light)
{
	ASSERT_TRUE(_renderer != nullptr, "No renderer has been set");
	ASSERT_TRUE(light->GetType() == LightType::Directional, "Light must be directional");
	_renderer->SetDirectionalLight(DirectionalLightData(light->GetColour(), light->GetDirection(), light->GetIntensity()));
	_directionalLight = light;
}

void SceneManager::SetAmbientLightColour(const Colour& colour)
{
	ASSERT_TRUE(_renderer != nullptr, "No renderer has been set");
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

SceneManager::SceneManager() :
	_sceneGraph(new SceneNode("root")),
	_ambientLightColour(Colour::White),
	_ambientLightIntensity(0.3f)
{
}

void SceneManager::PushRenderable(Actor* actor)
{
	_renderableActors.push_back(actor);
}

void SceneManager::SubmitSceneToRender()
{
	ASSERT_TRUE(_renderer != nullptr, "No renderer has been set");
  _renderer->SetAmbientLight(AmbientLightData(_ambientLightColour, _ambientLightIntensity));
	_renderer->SetDirectionalLight(DirectionalLightData(_directionalLight->GetColour(), _directionalLight->GetDirection(), _directionalLight->GetIntensity()));

	Frustrum camFrustrum(_camera->GetProjection() * _camera->GetView());
  
	for (auto renderableActor : _renderableActors)
	{
		auto actorbounds = renderableActor->GetBounds();
		if (renderableActor && camFrustrum.Intersects(actorbounds))
		{			
			_renderer->Push(renderableActor->GetComponent<Renderable>(), renderableActor->GetTransform(), actorbounds);
		}
	}
}
