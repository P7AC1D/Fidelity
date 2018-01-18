#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"

class OrbitalCamera;
class WorldObject;

namespace Rendering
{
class Renderer;
}

class SceneManager
{
public:
  SceneManager();

  WorldObject& CreateWorldObject(const std::string& name = std::string());

  inline void SetCamera(const std::shared_ptr<OrbitalCamera>& camera) { _camera = camera; }
  inline void SetAmbientColour(const Vector3& colour) { _ambientLight = colour; }
  void SetViewport(int32 renderWidth, int32 renderHeight);
  void SetClearColour(const Vector3& colour);

  inline const Vector3& GetAmbientLight() const { return _ambientLight; }

  void UpdateScene(uint32 dtMs);

private:
  void UpdateWorldObjects(uint32 dtMs);
  void DrawScene();

private:
  std::vector<WorldObject> _worldObjects;
  std::unique_ptr<Rendering::Renderer> _renderer;
  std::shared_ptr<OrbitalCamera> _camera;
  Vector3 _ambientLight;
};