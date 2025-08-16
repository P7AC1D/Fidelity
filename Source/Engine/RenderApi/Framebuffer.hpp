#pragma once
#include <memory>
#include <vector>
#include "Texture.hpp"

// Forward-declare RenderPass to keep headers light
class RenderPass;

/** A single attachment bound to a framebuffer slot. */
struct FramebufferAttachment
{
  std::shared_ptr<Texture> texture; ///< Texture view used as attachment image
};

/**
 * Describes the attachments and dimensions of a framebuffer.
 *
 * Note: This descriptor is trivially copyable to simplify usage. Depth/stencil
 * existence is expressed via hasDepthStencilAttachment.
 */
struct FramebufferDesc
{
  uint32 width = 0;                                    ///< Framebuffer width in pixels
  uint32 height = 0;                                   ///< Framebuffer height in pixels
  uint32 samples = 1;                                  ///< MSAA sample count
  std::vector<FramebufferAttachment> colorAttachments; ///< Color attachment list in binding order
  FramebufferAttachment depthStencilAttachment;        ///< Only valid when hasDepthStencilAttachment is true
  bool hasDepthStencilAttachment = false;              ///< Whether a depth/stencil attachment is present
};

/** Lightweight framebuffer holder used by render pass begin info. */
class Framebuffer
{
public:
  explicit Framebuffer(const FramebufferDesc &desc) : _desc(desc) {}
  virtual ~Framebuffer() = default;

  const FramebufferDesc &getDesc() const { return _desc; }
  uint32 getWidth() const { return _desc.width; }
  uint32 getHeight() const { return _desc.height; }
  uint32 getSamples() const { return _desc.samples; }

private:
  FramebufferDesc _desc;
};
