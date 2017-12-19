#include <string>

#include "Test3DApp.hpp"

int main(int argc, const char * argv[])
{
  ApplicationDesc applicationDesc;
  applicationDesc.Name = "SDL Application";
  applicationDesc.Width = 1366;
  applicationDesc.Height = 768;
  Application* sdlApp = new Test3DApp(applicationDesc);
  return sdlApp->Run();
}
