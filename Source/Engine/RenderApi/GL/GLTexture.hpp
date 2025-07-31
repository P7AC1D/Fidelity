#pragma once
#include "../../Core/Types.hpp"
#include "../Texture.hpp"

class GLTexture : public Texture
{
  friend class GLRenderDevice;

public:
  virtual ~GLTexture();

  uint32 getId() const;

  void writeData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData> &data) override;
  void writeData(uint32 mipLevel, uint32 face, uint32 xStart, uint32 xCount, uint32 yStart, uint32 yCount, uint32 zStart, uint32 zCount, void *data) override;
  void generateMips() override;

  // ResourceHandle interface implementation
  void *getNativeHandle() const override;
  bool isValid() const override;

protected:
  GLTexture(const TextureDesc &desc, bool gammaCorrected);

private:
  void Initialize();
  void Allocate();

private:
  uint32 _id;
};
