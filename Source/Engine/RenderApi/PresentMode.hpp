#pragma once
#include "../Core/Types.hpp"

/**
 * @brief Present modes for swapchain presentation.
 *
 * Maps to FIFO/MAILBOX/IMMEDIATE semantics on modern APIs. Where a mode is
 * unavailable, implementations should choose the closest behavior.
 */
enum class PresentMode : uint32
{
  Fifo = 0,      ///< Vsync on
  Mailbox = 1,   ///< Low-latency vsync
  Immediate = 2, ///< Vsync off
  Tearing = 3    ///< Tearing allowed
};
