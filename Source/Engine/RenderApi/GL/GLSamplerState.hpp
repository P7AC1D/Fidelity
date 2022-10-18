#pragma once
#include "../../Core/Types.hpp"
#include "../SamplerState.hpp"

class GLSamplerState : public SamplerState
{
  friend class GLRenderDevice;

public:
  ~GLSamplerState();

  uint32 getId() const { return _id; }

protected:
  GLSamplerState(const SamplerStateDesc &desc);

private:
  bool isInitialized() const { return _initalized; }
  void initialize();

  void setTextureAddressingMode(AddressingMode addressingMode);
  void setTextureMinMipFiltering(TextureFilteringMode minFilteringMode, TextureFilteringMode mipFilteringMode);
  void setTextureMagFiltering(TextureFilteringMode textureFilteringMode);
  void setBorderColour(Colour borderColour);

private:
  uint32 _id;
  bool _initalized;
};
