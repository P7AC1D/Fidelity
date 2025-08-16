#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include "ResourceSet.hpp"
#include "PushConstants.hpp"

/**
 * @brief Lightweight descriptor set layout abstractions.
 */

/**
 * @brief Describes a single binding within a descriptor set layout.
 *
 * A binding maps a shader-visible resource type to a numeric binding index and
 * indicates the shader stages that can access it. For buffer bindings, the
 * dynamicOffset flag indicates whether the binding supports dynamic offsets.
 */
struct DescriptorSetLayoutBinding
{
  /** Binding index within the descriptor set (e.g., 0, 1, 2...). */
  uint32_t binding{0};
  /** Resource type for this binding (uniform buffer, texture, etc.). */
  ResourceType type{ResourceType::UNIFORM_BUFFER};
  /** Number of arrayed resources for this binding (default 1). */
  uint32_t count{1};
  /** Shader stage visibility mask for this binding. */
  StageFlags stageFlags{StageFlags::AllGraphics};
  /** True if this binding supports dynamic offsets (buffers only). */
  bool dynamicOffset{false};
};

/**
 * @brief Lightweight descriptor set layout abstraction.
 *
 * Encapsulates an ordered list of bindings that define the resources available
 * in a descriptor set at a given set index within a PipelineLayout.
 */
class DescriptorSetLayout
{
public:
  DescriptorSetLayout() = default;
  explicit DescriptorSetLayout(std::vector<DescriptorSetLayoutBinding> bindings) : _bindings(std::move(bindings)) {}

  /**
   * @brief Access the binding descriptions that make up this layout.
   */
  const std::vector<DescriptorSetLayoutBinding> &getBindings() const { return _bindings; }

private:
  /** Ordered list of bindings for this descriptor set layout. */
  std::vector<DescriptorSetLayoutBinding> _bindings;
};
