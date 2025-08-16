#pragma once
#include <memory>
#include "../Core/Types.hpp"

struct GLFWwindow;

/**
 * @brief Handle to a presentation surface (window/system specific)
 */
class ISurface
{
public:
  virtual ~ISurface() = default;
  virtual void *getNativeHandle() const = 0;
  virtual uint32 getWidth() const = 0;
  virtual uint32 getHeight() const = 0;
};

/**
 * @brief Description required to create a swapchain
 */
struct SwapchainDesc
{
  uint32 width = 0;
  uint32 height = 0;
  uint32 imageCount = 2;  // double buffering by default
  bool sRGB = true;       // request sRGB backbuffer when possible
  uint32 presentMode = 0; // PresentMode as uint to avoid header coupling
};

class ISwapchain
{
public:
  virtual ~ISwapchain() = default;
  virtual uint32 acquireNextImage() = 0; // returns image index (0..N-1)
  virtual void present(uint32 imageIndex) = 0;
  virtual uint32 getImageCount() const = 0;
  virtual uint32 getWidth() const = 0;
  virtual uint32 getHeight() const = 0;
};
