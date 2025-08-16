#pragma once

#include "GL.hpp"
#include "../BlendState.hpp"
#include "../DepthStencilState.hpp"
#include "../Texture.hpp"
#include "../PrimitiveTopology.hpp"
#include "../ResourceState.hpp"

/**
 * @brief Converts a TextureType enumeration value to the corresponding OpenGL texture target.
 *
 * Maps engine-specific texture types to OpenGL texture target constants for use in
 * texture creation and binding operations.
 *
 * @param textureType The texture type to convert
 * @return GLenum The corresponding OpenGL texture target constant
 */
GLenum getTextureTargetFromType(TextureType textureType);

/**
 * @brief Converts a PrimitiveTopology enumeration value to the corresponding OpenGL primitive type.
 *
 * Maps engine-specific primitive topology types to OpenGL primitive constants for use in
 * rendering operations.
 *
 * @param topology The primitive topology to convert
 * @return GLenum The corresponding OpenGL primitive type constant
 */
GLenum getPrimitiveTopology(PrimitiveTopology topology);

/**
 * @brief Converts a StencilOperation enumeration value to the corresponding OpenGL stencil operation.
 *
 * Maps engine-specific stencil operations to OpenGL stencil operation constants.
 * Supports optional inversion of increment/decrement operations for two-sided stencil testing.
 *
 * @param stencilOperation The stencil operation to convert
 * @param invert Optional flag to invert increment/decrement operations (default: false)
 * @return GLenum The corresponding OpenGL stencil operation constant
 */
GLenum getStencilOp(StencilOperation stencilOperation, bool invert = false);

/**
 * @brief Converts a ComparisonFunction enumeration value to the corresponding OpenGL comparison function.
 *
 * Maps engine-specific comparison functions to OpenGL comparison function constants for use in
 * depth testing, stencil testing, and other comparison operations.
 *
 * @param func The comparison function to convert
 * @return GLenum The corresponding OpenGL comparison function constant
 */
GLenum getCompareFunc(ComparisonFunction func);

/**
 * @brief Converts a BlendFactor enumeration value to the corresponding OpenGL blend factor.
 *
 * Maps engine-specific blend factors to OpenGL blend factor constants for use in
 * alpha blending operations.
 *
 * @param factor The blend factor to convert
 * @return GLenum The corresponding OpenGL blend factor constant
 */
GLenum getBlendFactor(BlendFactor factor);

/**
 * @brief Converts a BlendOperation enumeration value to the corresponding OpenGL blend equation.
 *
 * Maps engine-specific blend operations to OpenGL blend equation constants for use in
 * alpha blending operations.
 *
 * @param op The blend operation to convert
 * @return GLenum The corresponding OpenGL blend equation constant
 */
GLenum getBlendOp(BlendOperation op);

/**
 * @brief Returns true if glMemoryBarrier is available at runtime (GL 4.2 or ARB_shader_image_load_store).
 */
bool glMemoryBarrierAvailable();

/**
 * @brief Maps engine MemoryDeps bits to OpenGL glMemoryBarrier bits (best-effort on older GL).
 *
 * If specific bits are not supported on the current GL version, this function
 * falls back to broader/safe bits or zero. Callers should use GL_ALL_BARRIER_BITS
 * when the returned value is zero but a barrier is still required.
 */
GLbitfield mapMemoryDepsToGL(uint32 memoryDeps);