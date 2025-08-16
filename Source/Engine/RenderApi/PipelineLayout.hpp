#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include "DescriptorSetLayout.hpp"
#include "PushConstants.hpp"

/**
 * @brief Pipeline bind point for descriptor set and pipeline layout operations.
 *
 * This enum identifies the context in which a pipeline layout is used when binding
 * descriptor sets and push constants. Graphics and compute are the currently
 * supported bind points.
 */
enum class PipelineBindPoint : uint32_t
{
  Graphics,
  Compute
};

/**
 * @brief Describes the interface between a pipeline and bound resources.
 *
 * PipelineLayout holds a sequence of descriptor set layouts and a collection of
 * push constant ranges that together define how shaders access resources.
 */
class PipelineLayout
{
public:
  /**
   * @brief Construct an empty pipeline layout with no descriptor sets or push constants.
   */
  PipelineLayout() = default;
  /**
   * @brief Construct a pipeline layout from descriptor set layouts and push constant ranges.
   * @param setLayouts Ordered list of descriptor set layouts (set index corresponds to position).
   * @param pushRanges Optional list of push constant ranges visible to stages.
   */
  PipelineLayout(std::vector<std::shared_ptr<DescriptorSetLayout>> setLayouts,
                 std::vector<PushConstantRange> pushRanges = {})
      : _setLayouts(std::move(setLayouts)), _pushConstantRanges(std::move(pushRanges)) {}

  /**
   * @brief Get the ordered descriptor set layouts in this pipeline layout.
   */
  const std::vector<std::shared_ptr<DescriptorSetLayout>> &getSetLayouts() const { return _setLayouts; }
  /**
   * @brief Get the defined push constant ranges for this pipeline layout.
   */
  const std::vector<PushConstantRange> &getPushConstantRanges() const { return _pushConstantRanges; }

private:
  /** Ordered list of descriptor set layouts (index is the set number). */
  std::vector<std::shared_ptr<DescriptorSetLayout>> _setLayouts;
  /** Push constant ranges visible to specific shader stages. */
  std::vector<PushConstantRange> _pushConstantRanges;
};
