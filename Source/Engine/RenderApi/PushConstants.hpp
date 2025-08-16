#pragma once
#include <cstdint>

/**
 * @brief Shader stage bitmask flags for binding visibility and push constants.
 */
enum class StageFlags : uint32_t
{
  None = 0,                                                   ///< No stages
  Vertex = 1u << 0,                                           ///< Vertex shader stage
  Fragment = 1u << 1,                                         ///< Fragment/pixel shader stage
  Geometry = 1u << 2,                                         ///< Geometry shader stage
  Hull = 1u << 3,                                             ///< Tessellation control (hull) shader stage
  Domain = 1u << 4,                                           ///< Tessellation evaluation (domain) shader stage
  Compute = 1u << 5,                                          ///< Compute shader stage
  AllGraphics = Vertex | Fragment | Geometry | Hull | Domain, ///< All graphics stages
  All = 0xFFFFFFFFu                                           ///< All stages including compute
};

/**
 * @brief Bitwise OR between StageFlags values.
 */
inline StageFlags operator|(StageFlags a, StageFlags b)
{
  return static_cast<StageFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
/**
 * @brief In-place bitwise OR.
 */
inline StageFlags &operator|=(StageFlags &a, StageFlags b)
{
  a = a | b;
  return a;
}
/**
 * @brief Returns true if any stage bit is set.
 */
inline bool any(StageFlags a)
{
  return static_cast<uint32_t>(a) != 0u;
}

/**
 * @brief Push constant range description.
 *
 * Defines a range within the push constant buffer that is visible to a set of
 * shader stages. Offsets and sizes are expressed in bytes.
 */
struct PushConstantRange
{
  StageFlags stages{StageFlags::None}; ///< Stage visibility mask
  uint32_t offset{0};                  ///< Byte offset from start of push constant storage
  uint32_t size{0};                    ///< Size in bytes of the range
};
