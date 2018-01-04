#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Maths/Vector3.hpp"

namespace Rendering
{
class CubeMap;
}

namespace SceneManagement
{
class OrbitalCamera;
class SceneNode;

class Scene
{
public:
  Scene(std::string name);
  ~Scene();

  void Update();

  void SetCamera(std::shared_ptr<OrbitalCamera> camera) { _camera = camera; }
  void SetAmbientLight(const Vector3& ambientColour) { _ambientLight = ambientColour; }
  void SetSkyBox(std::shared_ptr<Rendering::CubeMap> skyBox);

  std::string GetName() const { return _name; }
  std::shared_ptr<SceneNode> GetRootNode();
  std::shared_ptr<OrbitalCamera> GetCamera() { return _camera; }
  Vector3 GetAmbientLight() const { return _ambientLight; }
  std::shared_ptr<Rendering::CubeMap> GetSkyBox() const { return _skyBox; }

private:
  Scene(Scene&) = delete;
  Scene(Scene&&) = delete;
  Scene& operator=(Scene&) = delete;
  Scene& operator=(Scene&&) = delete;

  std::string _name;
  std::shared_ptr<SceneNode> _rootNode;
  std::shared_ptr<OrbitalCamera> _camera;
  std::shared_ptr<Rendering::CubeMap> _skyBox;

  Vector3 _ambientLight;
};
}