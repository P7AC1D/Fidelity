#pragma once
#include "../Maths/Colour.hpp"

enum class TextureAddressMode
{
  Wrap,
  Mirror,
  Clamp,
  Border
};

enum class TextureFilteringMode
{
  None,
  Point,
  Linear,
  Anisotropic
};

struct AddressingMode
{
  TextureAddressMode U = TextureAddressMode::Clamp;
  TextureAddressMode V = TextureAddressMode::Clamp;
  TextureAddressMode W = TextureAddressMode::Clamp;
};

struct SamplerStateDesc
{
  AddressingMode AddressingMode;
  TextureFilteringMode MinFiltering = TextureFilteringMode::None;
  TextureFilteringMode MaxFiltering = TextureFilteringMode::None;
  TextureFilteringMode MipFiltering = TextureFilteringMode::None;
  Colour BorderColour = Colour::Black;
};

class SamplerState
{
public:
  AddressingMode GetAddressingMode() const { return _desc.AddressingMode; }
  TextureFilteringMode GetMinFilteringMode() const { return _desc.MinFiltering; }
  TextureFilteringMode GetMagFilteringMode() const { return _desc.MaxFiltering; }
  TextureFilteringMode GetMipFilteringMode() const { return _desc.MipFiltering; }
  Colour GetBorderColour() const { return _desc.BorderColour; }

protected:
  SamplerState(const SamplerStateDesc& desc): _desc(desc) {}
  SamplerState(): _desc(SamplerStateDesc()) {}
  
protected:
  SamplerStateDesc _desc;
};
