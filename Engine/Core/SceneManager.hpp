#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Subsystem.hpp"
#include "Types.h"

class GameObject;
class RenderingSystem;

class SceneManager : public Subsystem<SceneManager>
{
public:
  const GameObject& CreateGameObject(const std::string& name);
  
  void UpdateScene(uint32 dtMs) const;
  
private:
  SceneManager();
  
private:
  std::unique_ptr<RenderingSystem> _renderingSystem;
  std::vector<GameObject> _gameObjects;
};
