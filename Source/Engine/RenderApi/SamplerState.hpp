#pragma once
#include "../Maths/Colour.hpp"
#include "ResourceHandle.hpp"

/**
 * @brief Texture UV addressing behavior for out-of-range coordinates.
 */
enum class TextureAddressMode
{
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder
};

/**
 * @brief Texture filtering options for minification/magnification and mipmaps.
 */
enum class TextureFilteringMode
{
  /// @brief Returns the value of the texture element that is nearest (in Manhattan distance) to the center of the pixel being textured.
  Nearest,
  /// @brief Returns the weighted average of the four texture elements that are closest to the center of the pixel being textured.
  Linear,
  /// @brief Chooses the mipmap that most closely matches the size of the pixel being textured and uses the Nearest criterion (the texture element nearest to the center of the pixel) to produce a texture value.
  NearestMipNearest,
  /// @brief Chooses the mipmap that most closely matches the size of the pixel being textured and uses the Linear criterion (a weighted average of the four texture elements that are closest to the center of the pixel) to produce a texture value.
  LinearMipNearest,
  /// @brief Chooses the two mipmaps that most closely match the size of the pixel being textured and uses the Nearest criterion (the texture element nearest to the center of the pixel) to produce a texture value from each mipmap. The final texture value is a weighted average of those two values.
  NearestMipLinear,
  /// @brief Chooses the two mipmaps that most closely match the size of the pixel being textured and uses the Linear criterion (a weighted average of the four texture elements that are closest to the center of the pixel) to produce a texture value from each mipmap. The final texture value is a weighted average of those two values.
  LinearMipLinear
};

/**
 * @brief Per-axis texture coordinate addressing modes.
 */
struct AddressingMode
{
  TextureAddressMode U = TextureAddressMode::ClampToEdge;
  TextureAddressMode V = TextureAddressMode::ClampToEdge;
  TextureAddressMode W = TextureAddressMode::ClampToEdge;
};

/**
 * @brief Sampler state description.
 */
struct SamplerStateDesc
{
  AddressingMode AddressingMode;
  /// @brief The texture minifying function is used whenever the pixel being textured maps to an area greater than one texture element.
  TextureFilteringMode MinFiltering = TextureFilteringMode::Nearest;
  /// @brief The texture magnification function is used when the pixel being textured maps to an area less than or equal to one texture element.
  TextureFilteringMode MagFiltering = TextureFilteringMode::Nearest;
  Colour BorderColour = Colour::Black;
  /// @brief Maximum anisotropy level for anisotropic filtering (1.0f = disabled, higher values = better quality)
  float MaxAnisotropy = 1.0f;
};

/**
 * @brief Immutable sampler state object.
 */
class SamplerState : public ResourceHandle
{
public:
  AddressingMode getAddressingMode() const { return _desc.AddressingMode; }
  TextureFilteringMode getMinFilteringMode() const { return _desc.MinFiltering; }
  TextureFilteringMode getMagFilteringMode() const { return _desc.MagFiltering; }
  Colour getBorderColour() const { return _desc.BorderColour; }
  float getMaxAnisotropy() const { return _desc.MaxAnisotropy; }

protected:
  SamplerState(const SamplerStateDesc &desc) : _desc(desc) {}
  SamplerState() : _desc(SamplerStateDesc()) {}

protected:
  SamplerStateDesc _desc;
};
