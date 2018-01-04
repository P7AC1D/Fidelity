#pragma once
#include <memory>

#include "../Engine/Core/Application.h"
#include "Test3D.h"

using namespace Platform;

int main()
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "SDL Application";
  applicationDesc.Width = 1366;
  applicationDesc.Height = 768;
  Application* sdlApp = new Test3D(applicationDesc);
  return sdlApp->Run();

}