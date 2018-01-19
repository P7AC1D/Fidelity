#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "WorldObject.h"

class OrbitalCamera;

namespace Rendering
{
class Renderer;
}

namespace Utility
{
class AssetManager;
}

class SceneManager
{
public:
  SceneManager(std::shared_ptr<Utility::AssetManager> _assetManager, std::shared_ptr<Rendering::Renderer> renderer);

  WorldObject& CreateObject(const std::string& name = std::string());
  WorldObject& LoadObjectFromFile(const std::string& filePath);

  inline void SetCamera(std::shared_ptr<OrbitalCamera> camera) { _camera = camera; }
  inline void SetAmbientColour(const Vector3& colour) { _ambientLight = colour; }

  inline const Vector3& GetAmbientLight() const { return _ambientLight; }

  void UpdateScene(uint32 dtMs);

private:
  void UpdateWorldObjects(uint32 dtMs);
  void SubmitSceneToRender();

private:
  std::vector<WorldObject> _worldObjects;
  std::shared_ptr<OrbitalCamera> _camera;
  std::shared_ptr<Utility::AssetManager> _assetManager;
  std::shared_ptr<Rendering::Renderer> _renderer;
  Vector3 _ambientLight;
};