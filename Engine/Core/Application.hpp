#pragma once
#include <string>

#include "Types.h"

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
  virtual void OnTick() {}
  
  std::string GetName() const { return _desc.Name; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }
  
  int32 Run();
  
protected:
  Application(const ApplicationDesc& desc);
  
private:
  bool Initialize();
  
private:
  ApplicationDesc _desc;
  SDL_Window* _window;
  bool _isRunning;
};
