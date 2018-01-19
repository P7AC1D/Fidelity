#include "SceneManager.h"

#include "../Components/PointLight.h"
#include "../Rendering/Renderer.h"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/WorldObject.h"
#include "../Utility/AssetManager.h"
#include "../Utility/ObjLoader.hpp"
#include "../Utility/StringUtil.h"
#include "OrbitalCamera.h"

static uint32 WorldObjectCount = 0;
static uint32 PointLightCount = 0;

using namespace Components;
using namespace Rendering;
using namespace Utility;

SceneManager::SceneManager(std::shared_ptr<AssetManager> _assetManager, std::shared_ptr<Renderer> renderer) :
  _ambientLight(Vector3::Identity),
  _assetManager(_assetManager),
  _renderer(renderer)
{
}

WorldObject& SceneManager::CreateObject(const std::string& name)
{
  std::string objectName(name); 
  if (name == std::string())
  {
    objectName = "WorldObject" + std::to_string(WorldObjectCount);
    WorldObjectCount++;
  }
  _worldObjects.emplace_back(objectName);
  return _worldObjects.back();
}

WorldObject& SceneManager::LoadObjectFromFile(const std::string& filePath)
{
  auto tokens = StringUtil::Split(filePath, '/');
  auto fileName = tokens.back();
  tokens.pop_back();

  auto renderable = ObjLoader::LoadFromFile(StringUtil::Join(tokens, '/'), fileName, *_assetManager.get());
  
  _worldObjects.emplace_back(fileName);
  auto& worldObject = _worldObjects.back();
  worldObject.SetRenderable(renderable);
  return worldObject;
}

void SceneManager::UpdateScene(uint32 dtMs)
{
  UpdateWorldObjects(dtMs);
  SubmitSceneToRender();
  _renderer->DrawScene(*_camera.get());
}

void SceneManager::UpdateWorldObjects(uint32 dtMs)
{
  for (auto& worldObject : _worldObjects)
  {
    worldObject.Update();
  }
}

void SceneManager::SubmitSceneToRender()
{
  for (auto& worldObject : _worldObjects)
  {
    auto renderable = worldObject.GetRenderable();
    if (renderable != nullptr)
    {
      auto transform = worldObject.GetTransform();
      _renderer->PushRenderable(renderable, transform);
    }

    auto lightComponent = worldObject.GetComponent("PointLight");
    if (lightComponent != nullptr)
    {
      auto pointLight = std::dynamic_pointer_cast<PointLight>(lightComponent);
      _renderer->PushPointLight(pointLight);
    }
  }
}