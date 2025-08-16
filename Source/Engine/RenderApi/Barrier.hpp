#pragma once
#include <memory>
#include "../Core/Types.hpp"
#include "ImageView.hpp"
#include "GpuBuffer.hpp"
#include "ResourceState.hpp"

// Buffer/Image barrier definitions

struct BufferBarrier
{
  GpuBuffer *buffer = nullptr;
  uint64 offset = 0;
  uint64 size = 0; // 0 => whole buffer
  ResourceState oldState = ResourceState::Undefined;
  ResourceState newState = ResourceState::General;
};

struct ImageSubresourceRangeEx
{
  // If view is provided, this range is relative to the view; otherwise absolute.
  uint32 baseMipLevel = 0;
  uint32 levelCount = 1;
  uint32 baseArrayLayer = 0;
  uint32 layerCount = 1;
};

struct ImageBarrier
{
  ImageView *view = nullptr; // preferred when targeting a subresource
  Texture *image = nullptr;  // alternative when binding the whole image
  ResourceState oldState = ResourceState::Undefined;
  ResourceState newState = ResourceState::General;
  ImageAspect aspect = ImageAspect::Color;
  ImageSubresourceRangeEx range{};
};
