#include "SceneManager.h"

#include <algorithm>

#include "Scene.h"

namespace SceneManagement
{
SceneManager::SceneManager()
{
  auto scene = std::make_shared<Scene>("main");
  _sceneCollection.push_back(scene);
  _activeScene = scene;
}

void SceneManager::UpdateActiveScene()
{
  _activeScene.lock()->Update();
}

void SceneManager::AddScene(std::shared_ptr<Scene> scene)
{
  _sceneCollection.push_back(scene);
}

void SceneManager::SetScene(const std::string& sceneName)
{
  auto iter = std::find_if(_sceneCollection.begin(), _sceneCollection.end(), [&sceneName](std::shared_ptr<Scene> scene) { return scene->GetName() == sceneName; });
  if (iter == _sceneCollection.end())
  {
    return;
  }
  _activeScene = *iter;
}

std::shared_ptr<Scene> SceneManager::GetActiveScene()
{
  return _activeScene.lock();
}
}