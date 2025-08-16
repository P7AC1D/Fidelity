#include <memory>
#include <iostream>

#include "../Engine/Core/Application.h"
#include "Sponza.h"

int main()
{
  std::cout << "Fidelity Engine - Sponza Demo" << std::endl;

  ApplicationDesc applicationDesc;
  applicationDesc.Name = "Sponza - Modern Component System";
  applicationDesc.Width = 1920;
  applicationDesc.Height = 1080;

  std::cout << "Running with Modern Component System (automatic dependency resolution)" << std::endl;
  Application *app = new SponzaModern(applicationDesc);

  int result = app->run();
  return result;
}
