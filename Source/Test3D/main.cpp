#include <memory>

#include "../Engine/Core/Application.h"
#include "Test3D.h"

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "Test3D";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;
  Application *sdlApp = new Test3D(applicationDesc);
  return sdlApp->run();
}
