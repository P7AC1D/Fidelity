#pragma once
#include "../../Core/Types.hpp"
#include "../SamplerState.hpp"

class GLSamplerState : public SamplerState
{
  friend class GLRenderDevice;
public:
  ~GLSamplerState();
  
  uint32 GetId() const { return _id; }
  
protected:
  GLSamplerState(const SamplerStateDesc& desc);
  
private:
  bool IsInitialized() const { return _initalized; }
  void Initialize();
  
  void SetTextureAddressingMode(AddressingMode addressingMode);
  void SetTextureMinMipFiltering(TextureFilteringMode minFilteringMode, TextureFilteringMode mipFilteringMode);
  void SetTextureMagFiltering(TextureFilteringMode textureFilteringMode);
  
private:
  uint32 _id;
  bool _initalized;
};
