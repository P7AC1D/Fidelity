#pragma once
#include "ResourceHandle.hpp"
#include "Texture.hpp"

enum class ImageAspect : uint8
{
  Color = 1,
  Depth = 2,
  Stencil = 4,
  DepthStencil = 6,
};

struct ImageSubresourceRange
{
  uint32 baseMipLevel = 0;
  uint32 levelCount = 1;
  uint32 baseArrayLayer = 0;
  uint32 layerCount = 1; // for cube arrays, layerCount = 6 * cubeCount
};

struct ImageViewDesc
{
  Texture *image = nullptr;
  TextureFormat viewFormat; // may reinterpret within compatible class
  ImageAspect aspect = ImageAspect::Color;
  ImageSubresourceRange range{};
};

class ImageView : public ResourceHandle
{
public:
  explicit ImageView(const ImageViewDesc &desc) : _desc(desc) {}
  virtual ~ImageView() = default;

  const ImageViewDesc &getDesc() const { return _desc; }

  // Return the underlying native handle for this view (may alias the image handle)
  void *getNativeHandle() const override
  {
    return _desc.image ? _desc.image->getNativeHandle() : nullptr;
  }
  bool isValid() const override
  {
    return _desc.image != nullptr && _desc.image->isValid();
  }

protected:
  ImageViewDesc _desc;
};
