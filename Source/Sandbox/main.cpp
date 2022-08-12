#include <memory>

#include "../Engine/Core/Application.h"
#include "Sandbox.h"

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "SDL Application";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;
  Application* sdlApp = new Sandbox(applicationDesc);
  return sdlApp->Run();
}
