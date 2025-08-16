#include "GLSwapchain.hpp"
#include <GLFW/glfw3.h>

static void ApplyPresentModeToGLFW(PresentMode mode)
{
  switch (mode)
  {
  case PresentMode::Fifo:
    glfwSwapInterval(1);
    break;
  case PresentMode::Mailbox:
    glfwSwapInterval(1);
    break; // best-effort on GL
  case PresentMode::Immediate:
    glfwSwapInterval(0);
    break;
  case PresentMode::Tearing:
    glfwSwapInterval(0);
    break; // best-effort on GL
  }
}

GLSwapchain::GLSwapchain(std::shared_ptr<GLSurface> surface, const SwapchainDesc &desc, PresentMode mode)
    : _surface(std::move(surface)), _presentMode(mode)
{
  (void)desc;
  // Apply swap interval on the current context
  ApplyPresentModeToGLFW(_presentMode);
}

uint32 GLSwapchain::acquireNextImage()
{
  return 0u; // default FBO acts as a single image
}

void GLSwapchain::present(uint32 /*imageIndex*/)
{
  GLFWwindow *win = _surface ? _surface->getWindow() : nullptr;
  if (win)
  {
    glfwSwapBuffers(win);
  }
}
