#pragma once
#include "../../Core/Types.hpp"
#include "../SamplerState.hpp"

class GLSamplerState : public SamplerState
{
  friend class GLRenderDevice;

public:
  ~GLSamplerState();

  void *getNativeHandle() const override;
  bool isValid() const override;

protected:
  GLSamplerState(const SamplerStateDesc &desc);

private:
  bool isInitialized() const { return _initalized; }
  void initialize();

  void setTextureAddressingMode(AddressingMode addressingMode);
  void setTextureMinMipFiltering(TextureFilteringMode minFilteringMode);
  void setTextureMagFiltering(TextureFilteringMode textureFilteringMode);
  void setBorderColour(Colour borderColour);
  void setAnisotropicFiltering(float maxAnisotropy);

private:
  uint32 _id;
  bool _initalized;
};
