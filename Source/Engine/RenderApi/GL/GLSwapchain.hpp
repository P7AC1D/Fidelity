#pragma once
#include <memory>
#include "../Surface.hpp"
#include "../PresentMode.hpp"
#include "GLSurface.hpp"

/**
 * @brief OpenGL swapchain wrapper backed by the default framebuffer.
 *
 * On OpenGL/GLFW, presentation is performed by swapping the default framebuffer
 * via glfwSwapBuffers. Present mode is approximated using glfwSwapInterval.
 */
class GLSwapchain final : public ISwapchain
{
public:
  /**
   * Construct a GL swapchain for a given surface.
   * @param surface   The GLFW-based surface to present to.
   * @param desc      Swapchain description (width/height/imageCount/sRGB/presentMode).
   * @param mode      Present mode (mapped to swap interval under GL).
   */
  GLSwapchain(std::shared_ptr<GLSurface> surface, const SwapchainDesc &desc, PresentMode mode);

  // ISwapchain
  uint32 acquireNextImage() override; // returns 0 for default FBO
  void present(uint32 imageIndex) override;
  uint32 getImageCount() const override { return 1; }
  uint32 getWidth() const override { return _surface->getWidth(); }
  uint32 getHeight() const override { return _surface->getHeight(); }

private:
  std::shared_ptr<GLSurface> _surface;
  PresentMode _presentMode;
};
