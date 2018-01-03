#include "SceneManager.hpp"

#include "Camera.hpp"
#include "GameObject.hpp"
#include "RenderingSystem.hpp"

const GameObject& SceneManager::CreateGameObject(const std::string& name)
{
  GameObject gameObject(name);
  _gameObjects.push_back(std::move(gameObject));
  return _gameObjects.back();
}

void SceneManager::UpdateScene(uint32 dtMs) const
{
  for (auto& gameObject : _gameObjects)
  {
    gameObject.Update(dtMs);
  }
}

