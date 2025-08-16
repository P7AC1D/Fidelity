#pragma once
#include <memory>
#include <vector>
#include "Texture.hpp"

/**
 * RenderPass abstractions and begin info used to delineate rendering scopes.
 *
 * This header provides a minimal, Vulkan-like model suitable for explicit APIs.
 * The current implementation supports a single subpass.
 */

/** How an attachment should be treated at render pass begin. */
enum class AttachmentLoadOp
{
  Load,
  Clear,
  DontCare
};

/** Whether and how an attachment is stored at render pass end. */
enum class AttachmentStoreOp
{
  Store,
  DontCare
};

/**
 * Placeholder resource states to describe initial/final attachment states.
 */
enum class TextureInitialFinalState
{
  Undefined,
  ColorAttachment,
  DepthStencilAttachment,
  Present
};

/** Describes an attachment format and load/store semantics. */
struct AttachmentDescription
{
  TextureFormat format;
  uint32 samples = 1;
  AttachmentLoadOp loadOp = AttachmentLoadOp::Clear;
  AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
  AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::DontCare;
  AttachmentStoreOp stencilStoreOp = AttachmentStoreOp::DontCare;
  TextureInitialFinalState initialState = TextureInitialFinalState::Undefined;
  TextureInitialFinalState finalState = TextureInitialFinalState::Undefined;
};

/** Single-subpass description: color attachments plus optional depth/stencil. */
struct SubpassDescription
{
  std::vector<uint32> colorAttachmentIndices;
  int32 depthStencilAttachmentIndex = -1;
};

/**
 * Opaque render pass object holding attachment and subpass metadata.
 * For now, a single subpass is supported.
 */
class RenderPass
{
public:
  RenderPass() = default;
  explicit RenderPass(const std::vector<AttachmentDescription> &attachments,
                      const SubpassDescription &subpass)
      : _attachments(attachments), _subpass(subpass) {}

  const std::vector<AttachmentDescription> &getAttachments() const { return _attachments; }
  const SubpassDescription &getSubpass() const { return _subpass; }

private:
  std::vector<AttachmentDescription> _attachments;
  SubpassDescription _subpass;
};

/** RGBA clear value for color attachments. */
struct ClearColorValue
{
  float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
};

/** Depth/stencil clear values. */
struct ClearDepthStencilValue
{
  float depth = 1.0f;
  int32 stencil = 0;
};

/** Render area rectangle in framebuffer coordinates. */
struct RenderArea
{
  int32 x = 0;
  int32 y = 0;
  uint32 width = 0;
  uint32 height = 0;
};

class Framebuffer; // fwd

/**
 * Parameters required to begin a render pass.
 *
 * - renderPass: The render pass definition.
 * - framebuffer: Target attachments and dimensions; null targets the default backbuffer.
 * - renderArea: If width/height are zero, the framebuffer extents are used.
 * - clearColors: One entry per color attachment referenced by the subpass.
 * - clearDepthStencil: Optional depth/stencil clear value.
 */
struct RenderPassBeginInfo
{
  std::shared_ptr<RenderPass> renderPass;
  std::shared_ptr<Framebuffer> framebuffer;                  // target attachments & dimensions
  RenderArea renderArea{};                                   // if zero width/height, use framebuffer extents
  std::vector<ClearColorValue> clearColors;                  // one per color attachment referenced by subpass
  std::unique_ptr<ClearDepthStencilValue> clearDepthStencil; // optional
};
