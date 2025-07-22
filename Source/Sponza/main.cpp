#include <memory>
#include <iostream>

#include "../Engine/Core/Application.h"
#include "Sponza.h"

// Configuration flag to choose which version to run
#define USE_MODERN_COMPONENT_SYSTEM 1

int main()
{
  std::cout << "Fidelity Engine - Sponza Demo" << std::endl;
  
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "Sponza - Modern Component System";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;

#if USE_MODERN_COMPONENT_SYSTEM
  std::cout << "Running with Modern Component System (automatic dependency resolution)" << std::endl;
  Application *app = new SponzaModern(applicationDesc);
#else
  std::cout << "Running with Legacy Component System" << std::endl;
  Application *app = new Sponza(applicationDesc);
#endif

  int result = app->run();
  delete app;
  return result;
}
