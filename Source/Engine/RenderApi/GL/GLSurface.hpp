#pragma once
#include <memory>
#include "../Surface.hpp"

struct GLFWwindow;

/**
 * @brief OpenGL/GLFW presentation surface wrapper.
 *
 * Wraps a GLFWwindow* as an ISurface to unify presentation across backends.
 * Stores the last-known framebuffer size for convenience.
 */
class GLSurface final : public ISurface
{
public:
  /**
   * Construct from an existing GLFW window and its framebuffer size.
   * @param win Non-owning GLFWwindow pointer.
   * @param w   Framebuffer width in pixels.
   * @param h   Framebuffer height in pixels.
   */
  explicit GLSurface(GLFWwindow *win, uint32 w, uint32 h) : _window(win), _w(w), _h(h) {}

  // ISurface
  void *getNativeHandle() const override { return (void *)_window; }
  uint32 getWidth() const override { return _w; }
  uint32 getHeight() const override { return _h; }

  // GL-specific
  GLFWwindow *getWindow() const { return _window; }
  void setSize(uint32 w, uint32 h)
  {
    _w = w;
    _h = h;
  }

private:
  GLFWwindow *_window;
  uint32 _w, _h;
};
