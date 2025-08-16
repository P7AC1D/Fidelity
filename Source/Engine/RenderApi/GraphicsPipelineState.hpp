#pragma once
#include <memory>
#include <vector>
#include "../Core/Types.hpp"
#include "BlendState.hpp"
#include "DepthStencilState.hpp"
#include "RasterizerState.hpp"
#include "VertexLayout.hpp"
#include "VertexInput.hpp"
#include "Shader.hpp"
#include "ShaderParams.hpp"
#include "Texture.hpp"
#include "PipelineLayout.hpp"
#include "PrimitiveTopology.hpp"

/**
 * @brief Dynamic state mask for graphics pipelines.
 */
enum class DynamicStateBits : uint32
{
  None = 0,
  Viewport = 1 << 0,
  Scissor = 1 << 1,
  PrimitiveTopology = 1 << 2,
};

inline DynamicStateBits operator|(DynamicStateBits a, DynamicStateBits b)
{
  return static_cast<DynamicStateBits>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

/**
 * @brief Description for a graphics pipeline state object.
 *
 * Contains shader programs, fixed-function state, vertex input, layout and
 * render target compatibility fields. Backends consume this during pipeline
 * creation and command buffer binding.
 */
struct GraphicsPipelineStateDesc
{
  // Programmable shaders
  std::shared_ptr<Shader> VS;
  std::shared_ptr<Shader> FS;
  std::shared_ptr<Shader> GS; // optional
  std::shared_ptr<Shader> HS; // optional
  std::shared_ptr<Shader> DS; // optional

  // Fixed-function state
  std::shared_ptr<RasterizerState> Rasterizer;
  std::shared_ptr<DepthStencilState> DepthStencil;
  std::shared_ptr<BlendState> Blend;

  // Vertex input and topology
  std::shared_ptr<VertexLayout> VertexLayoutDef;
  // Modern vertex input description (optional)
  std::vector<VertexBindingDesc> VertexBindings;     // binding descriptions
  std::vector<VertexAttributeDesc> VertexAttributes; // attribute descriptions
  PrimitiveTopology Topology = PrimitiveTopology::TriangleList;

  // Pipeline layout and optional shader parameter reflection
  std::shared_ptr<PipelineLayout> Layout;              // optional
  std::shared_ptr<ShaderParams> ShaderParamReflection; // optional

  // Render target compatibility (formats and sample count)
  std::vector<TextureFormat> ColorAttachmentFormats; // binding order
  bool HasDepthStencil = false;
  uint32 Samples = 1;                                // MSAA sample count

  // Dynamic state control
  DynamicStateBits DynamicStateMask = DynamicStateBits::None;
};

/**
 * @brief Lightweight graphics pipeline object holding a description.
 *
 * Backends may subclass this for specialized behavior.
 */
class GraphicsPipelineState
{
public:
  explicit GraphicsPipelineState(const GraphicsPipelineStateDesc &desc) : _desc(desc) {}
  virtual ~GraphicsPipelineState() = default;

  // Accessors
  const GraphicsPipelineStateDesc &getDesc() const { return _desc; }

  // Convenience getters for common fields
  const std::shared_ptr<Shader> &getVS() const { return _desc.VS; }
  const std::shared_ptr<Shader> &getFS() const { return _desc.FS; }
  const std::shared_ptr<Shader> &getGS() const { return _desc.GS; }
  const std::shared_ptr<Shader> &getHS() const { return _desc.HS; }
  const std::shared_ptr<Shader> &getDS() const { return _desc.DS; }
  const std::shared_ptr<RasterizerState> &getRasterizer() const { return _desc.Rasterizer; }
  const std::shared_ptr<DepthStencilState> &getDepthStencil() const { return _desc.DepthStencil; }
  const std::shared_ptr<BlendState> &getBlend() const { return _desc.Blend; }
  const std::shared_ptr<VertexLayout> &getVertexLayout() const { return _desc.VertexLayoutDef; }
  const std::vector<VertexBindingDesc> &getVertexBindings() const { return _desc.VertexBindings; }
  const std::vector<VertexAttributeDesc> &getVertexAttributes() const { return _desc.VertexAttributes; }
  PrimitiveTopology getTopology() const { return _desc.Topology; }
  const std::shared_ptr<PipelineLayout> &getLayout() const { return _desc.Layout; }
  const std::shared_ptr<ShaderParams> &getShaderParams() const { return _desc.ShaderParamReflection; }

private:
  GraphicsPipelineStateDesc _desc;
};
