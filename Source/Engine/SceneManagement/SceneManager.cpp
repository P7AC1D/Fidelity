#include "SceneManager.h"

#include "../Components/Component.h"
#include "../Rendering/Renderer.h"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/WorldObject.h"
#include "../Utility/AssetManager.h"
#include "../Utility/StringUtil.h"
#include "OrbitalCamera.h"
#include "WorldObject.h"

static uint32 WorldObjectCount = 0;
static uint32 LightCount = 0;

using namespace Rendering;
using namespace Utility;

SceneManager::SceneManager(std::shared_ptr<AssetManager> _assetManager, std::shared_ptr<Renderer> renderer) :
  _assetManager(_assetManager),
  _renderer(renderer),
  _ambientLight(Colour::Black)
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

std::shared_ptr<WorldObject> SceneManager::LoadObjectFromFile(const std::string& filePath)
{
  auto tokens = StringUtil::Split(filePath, '/');
  auto fileName = tokens.back();
  tokens.pop_back();

  auto renderable = _assetManager->GetRenderable(StringUtil::Join(tokens, '/'), fileName);
  
  _worldObjects.emplace_back(std::make_shared<WorldObject>(fileName));
  auto worldObject = _worldObjects.back();
  worldObject->AttachRenderable(renderable);
  return worldObject;
}

Light& SceneManager::CreateLight(LightType lightType, const std::string& name)
{
  std::string objectName(name);
  if (name == std::string())
  {
    objectName = "Light" + std::to_string(LightCount);
    LightCount++;
  }
  _lights.emplace_back(lightType, objectName);
  return _lights.back();
}

void SceneManager::UpdateScene(uint32 dtMs)
{
  UpdateWorldObjects(dtMs);
  SubmitSceneToRender();
  _renderer->SetAmbientLight(_ambientLight);
  _renderer->DrawScene(*_camera.get());
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
  _renderer->SetSkyBox(_skyBox);

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

  for (auto& light : _lights)
  {
    switch (light.GetType())
    {
      case LightType::Point:
        _renderer->PushPointLight(light);
        break;
      case LightType::Directional:
        _renderer->PushDirectionalLight(light);
        break;
      case LightType::Spot:
        break;
    }
  }
}
