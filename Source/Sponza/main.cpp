#include <memory>

#include "../Engine/Core/Application.h"
#include "Sponza.h"

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "SDL Application";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;
  Application *sdlApp = new Sponza(applicationDesc);
  return sdlApp->Run();
}