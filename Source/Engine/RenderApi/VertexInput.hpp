#pragma once
#include <cstdint>
#include <vector>
#include "VertexLayout.hpp" // Reuse SemanticFormat for attribute formats

/**
 * @brief Vertex input rate for a binding
 */
enum class VertexInputRate : uint32
{
  PerVertex = 0,
  PerInstance = 1,
};

/**
 * @brief Describes a single vertex buffer binding.
 * binding: binding index
 * stride: byte stride between consecutive elements
 * inputRate: per-vertex or per-instance data step
 */
struct VertexBindingDesc
{
  uint32 binding = 0;
  uint32 stride = 0; // If 0, stride may be inferred by the backend
  VertexInputRate inputRate = VertexInputRate::PerVertex;
};

/**
 * @brief Describes a single vertex attribute.
 * location: shader location/semantic index
 * binding: which binding this attribute pulls from
 * format: data format/components (reuses SemanticFormat)
 * offset: byte offset within the element
 */
struct VertexAttributeDesc
{
  uint32 location = 0;
  uint32 binding = 0;
  SemanticFormat format = SemanticFormat::Float3;
  uint32 offset = 0;
  // Optional per-attribute stride override. When 0, binding.stride is used;
  // when binding.stride is also 0, backends may infer a tight stride from attributes in the same binding.
  uint32 stride = 0;
  bool normalized = false; // compatibility; ignored for float formats
};
