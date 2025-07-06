#include <memory>

#include "../Engine/Core/Application.h"
#include "CullingTest.h"

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "CullingTest";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;
  Application *sdlApp = new CullingTest(applicationDesc);
  return sdlApp->run();
}
