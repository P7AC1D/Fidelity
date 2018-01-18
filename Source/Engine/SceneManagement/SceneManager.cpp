#include "SceneManager.h"

#include "../Rendering/Renderer.h"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/WorldObject.h"

static uint32 WorldObjectCount = 0;
static uint32 PointLightCount = 0;

using namespace Rendering;

SceneManager::SceneManager() :
  _ambientLight(Vector3::Identity)
{
}

WorldObject& SceneManager::CreateWorldObject(const std::string& name)
{
  std::string objectName(name); 
  if (name == std::string())
  {
    objectName = "WorldObject" + std::to_string(WorldObjectCount);
    WorldObjectCount++;
  }
  _worldObjects.emplace_back(objectName);
}

void SceneManager::SetViewport(int32 renderWidth, int32 renderHeight)
{
  _renderer->SetViewport(renderWidth, renderHeight);
}

void SceneManager::SetClearColour(const Vector3& colour)
{
  _renderer->SetClearColour(colour);
}

void SceneManager::UpdateScene(uint32 dtMs)
{

}

void SceneManager::UpdateWorldObjects(uint32 dtMs)
{
  for (auto& worldObject : _worldObjects)
  {
    worldObject.Update();
  }
}

void SceneManager::DrawScene()
{

}