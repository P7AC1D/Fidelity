#pragma once
#include <cstdint>

/**
 * @brief Centralized render API limits and reserved bindings.
 *
 * This header provides common limits and reserved indices used across backends
 * to keep behavior consistent and easy to discover. Backend-specific nuances
 * are noted in comments where relevant.
 */
namespace RenderLimits
{
  /**
   * @brief Maximum bytes supported by push-constant emulation on backends without native support.
   */
  constexpr std::uint32_t PushConstantsMaxBytes = 256u;

  /**
   * @brief Reserved binding index used by select backends to emulate push constants.
   */
  constexpr std::uint32_t GLPushConstantsBinding = 15u;
}
