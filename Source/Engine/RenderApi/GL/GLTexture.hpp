#pragma once
#include "../../Core/Types.hpp"
#include "../Texture.hpp"

class GLTexture : public Texture
{
public:
  virtual ~GLTexture();
  
  void WriteData(uint32 mipLevel, uint32 face, const ImageData& data) override;
  void GenerateMips() override;
  
  uint32 GetId() const { return _id; }
  
protected:
  GLTexture(const TextureDesc& desc);

private:
  void Initialize();
  void Allocate();
  
private:
  uint32 _id;
};
