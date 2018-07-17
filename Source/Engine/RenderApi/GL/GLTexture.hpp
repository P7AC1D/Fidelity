#pragma once
#include "../../Core/Types.hpp"
#include "../Texture.hpp"

class GLTexture : public Texture
{
  friend class GLRenderDevice;
  
public:
  virtual ~GLTexture();
  
  void WriteData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData>& data) override;
  void GenerateMips() override;
  
  uint32 GetId() const { return _id; }
  
protected:
  GLTexture(const TextureDesc& desc, bool gammaCorrected);

private:
  void Initialize();
  void Allocate();
  
private:
  uint32 _id;
};
