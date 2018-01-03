#pragma once
#include <memory>
#include <string>

#include "Types.h"
#include "SceneManager.hpp"

struct SDL_Window;

struct ApplicationDesc
{
  std::string Name;
  uint32 Width;
  uint32 Height;
};

class Application
{
public:
  virtual ~Application();
  
  virtual void OnStart() {}
  virtual void OnInput() {}
  virtual void OnDraw() {}
  virtual void OnTick(uint32 dtMs) {}
  
  std::string GetName() const { return _desc.Name; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }
  
  int32 Run();
  
protected:
  Application(const ApplicationDesc& desc);
  
private:
  bool Initialize();
  uint32 GetTickDuration();
  
private:
  const SceneManager& _sceneManager;
  ApplicationDesc _desc;
  SDL_Window* _window;
  bool _isRunning;
  uint32 _lastTimeInMs;
  uint32 _currentTimeInMs;
};
