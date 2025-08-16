#pragma once
#include <memory>
#include "../Core/Types.hpp"
#include "Shader.hpp"
#include "PipelineLayout.hpp"

/**
 * @brief Description for a compute pipeline state object.
 */
struct ComputePipelineStateDesc
{
  /** Compute shader program. */
  std::shared_ptr<Shader> CS;
  /** Optional pipeline layout that defines descriptor/resource bindings. */
  std::shared_ptr<PipelineLayout> Layout;
};

/**
 * @brief Lightweight compute pipeline object holding a description.
 *
 * Backends may subclass this for specialization.
 */
class ComputePipelineState
{
public:
  explicit ComputePipelineState(const ComputePipelineStateDesc &desc) : _desc(desc) {}
  virtual ~ComputePipelineState() = default;

  /** Returns the underlying description used to create this pipeline. */
  const ComputePipelineStateDesc &getDesc() const { return _desc; }
  /** Returns the compute shader. */
  const std::shared_ptr<Shader> &getCS() const { return _desc.CS; }
  /** Returns the pipeline layout. */
  const std::shared_ptr<PipelineLayout> &getLayout() const { return _desc.Layout; }

private:
  ComputePipelineStateDesc _desc;
};
