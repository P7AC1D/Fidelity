#pragma once

/**
 * @brief Graphics primitive assembly topology.
 *
 * Describes how the GPU interprets a sequence of vertices when drawing.
 * Backends convert these values to their native API enums at bind time.
 */
enum class PrimitiveTopology
{
  /** Independent triangles, 3 vertices per triangle. */
  TriangleList,
  /** Connected triangle strip sharing two vertices between adjacent triangles. */
  TriangleStrip,
  /** Standalone points. */
  PointList,
  /** Independent line segments, 2 vertices per line. */
  LineList,
  /** Patches for tessellation stages (backend-dependent control points). */
  PatchList
};
