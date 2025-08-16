#pragma once
#include <memory>
#include "../ComputePipelineState.hpp"

/**
 * @brief OpenGL 4.1 compute pipeline wrapper.
 *
 * Compute is not supported for GL 4.1 in this project; the object exists only
 * to carry the description and support a unified API surface.
 */
class GLComputePipelineState : public ComputePipelineState
{
public:
  explicit GLComputePipelineState(const ComputePipelineStateDesc &desc) : ComputePipelineState(desc) {}
  ~GLComputePipelineState() override = default;
};
