#pragma once
#include <memory>
#include "../GraphicsPipelineState.hpp"

/**
 * @brief OpenGL 4.1 graphics pipeline wrapper.
 *
 * This class exists to mirror the backend-specific pipeline type. In GL 4.1,
 * pipeline creation does not compile or bind state; GL state is applied during
 * command buffer binding.
 */
class GLGraphicsPipelineState : public GraphicsPipelineState
{
public:
  explicit GLGraphicsPipelineState(const GraphicsPipelineStateDesc &desc) : GraphicsPipelineState(desc) {}
  ~GLGraphicsPipelineState() override = default;
};
