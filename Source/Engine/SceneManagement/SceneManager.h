#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"
#include "Light.h"
#include "SkyBox.hpp"

class OrbitalCamera;
class WorldObject;

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

  std::shared_ptr<WorldObject> CreateObject(const std::string& name = std::string());  
  std::shared_ptr<WorldObject> LoadObjectFromFile(const std::string& filePath);
  Light& CreateLight(LightType lightType, const std::string& name = std::string());

  inline void SetCamera(std::shared_ptr<OrbitalCamera> camera) { _camera = camera; }
  inline void SetSkyBox(std::shared_ptr<SkyBox> skyBox) { _skyBox = skyBox; }
  inline void SetAmbientLight(const Colour& colour) { _ambientLight = colour; }

  inline const Colour& GetAmbientLight() const { return _ambientLight; }

  void UpdateScene(uint32 dtMs);

private:
  void UpdateWorldObjects(uint32 dtMs);
  void SubmitSceneToRender();

private:
  std::vector<std::shared_ptr<WorldObject>> _worldObjects;
  std::vector<Light> _lights;
  std::shared_ptr<OrbitalCamera> _camera;
  std::shared_ptr<Utility::AssetManager> _assetManager;
  std::shared_ptr<Rendering::Renderer> _renderer;
  std::shared_ptr<SkyBox> _skyBox;
  Colour _ambientLight;
};