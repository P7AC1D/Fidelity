#pragma once
#include <memory>

#include "../Engine/Core/Application.h"
#include "../Engine/SceneManagement/WorldObject.h"

class GuiCaption;
class GuiPanel;
class Light;
class OrbitalCamera;
class TextOverlay;
class WorldObject;

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc& desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  std::shared_ptr<OrbitalCamera> _camera;
  std::shared_ptr<WorldObject> _object;
  Light* _light;
  std::shared_ptr<TextOverlay> _onScreenFpsCounter;
  std::shared_ptr<GuiPanel> _testPanel;
  std::shared_ptr<GuiPanel> _testChildPanel;
  std::shared_ptr<GuiCaption> _fpsCounter;
};
