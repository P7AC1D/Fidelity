#include <memory>

#include "../Engine/Core/Application.h"
#include "Test3D.h"

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "SDL Application";
  applicationDesc.Width = 720 * 2;
  applicationDesc.Height = 480 * 2;
  Application* sdlApp = new Test3D(applicationDesc);
  return sdlApp->Run();

}
