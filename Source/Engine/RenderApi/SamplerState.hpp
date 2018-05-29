#pragma once

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
};

class SamplerState
{
  friend class GLRenderDevice;
  
public:
  AddressingMode GetAddressingMode() const { return _desc.AddressingMode; }
  TextureFilteringMode GetMinFilteringMode() const { return _desc.MinFiltering; }
  TextureFilteringMode GetMaxFilteringMode() const { return _desc.MaxFiltering; }

protected:
  SamplerState(const SamplerStateDesc& desc): _desc(desc) {}
  
protected:
  SamplerStateDesc _desc;
};
