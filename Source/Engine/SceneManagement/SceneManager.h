#pragma once
#include <memory>
#include <string>
#include <vector>

namespace SceneManagement
{
class Scene;

class SceneManager
{
public:
  SceneManager();

  void UpdateActiveScene();

  void AddScene(std::shared_ptr<Scene> scene);
  void SetScene(const std::string& sceneName);

  std::shared_ptr<Scene> GetActiveScene();

private:
  std::vector<std::shared_ptr<Scene>> _sceneCollection;
  std::weak_ptr<Scene> _activeScene;
};
}