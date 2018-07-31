#pragma once
#include <memory>
#include "../Image/ImageData.hpp"

enum class TextureFormat
{
  /// 8-bit red channel as unsigned bytes.
  R8,
  /// 8-bit red and green channels stored as unsigned bytes.
  RG8,
  /// 8-bit red, green and blue channels stored as unsigned bytes.
  RGB8,
  /// 8-bit red, green, blue and alpha channels stored as unsigned bytes.
  RGBA8,
  /// 16-bit red, green and blue channels stored as a signed floats.
  RGB16F,
  /// 16-bit red, green, blue and alpha channels stored as a signed floats.
  RGBA16F,
  /// 32-bit depth channel stored as unsigned bytes.
  D32,
  /// 24-bit depth channel with an 8 bit stencil channel stored as unsigned bytes.
  D24S8
};

enum class TextureUsage
{
  Default,
  RenderTarget,
  Depth,
  DepthStencil,
};

enum class TextureType
{
  Texture1D,
  Texture1DArray,
  Texture2D,
  Texture2DArray,
  TextureCube,
  Texture3D,
};

struct TextureDesc
{
  TextureFormat Format;
  TextureType Type;
  uint32 Width;
  uint32 Height;
  uint32 Depth = 1;
  uint32 Count = 1;
  uint32 MipLevels = 1;
  TextureUsage Usage = TextureUsage::Default;
};

class Texture
{
public:
  Texture(const TextureDesc& desc, bool gammaCorrected): _desc(desc), _gammaCorrected(gammaCorrected), _isInitialized(false) {}
  
  TextureType GetTextureType() const { return _desc.Type; }
  uint32 GetWidth() const { return _desc.Width; }
  uint32 GetHeight() const { return _desc.Height; }
  uint32 GetDepth() const { return _desc.Depth; }

  const TextureDesc& GetDesc() const { return _desc; }
  bool IsInitialized() const { return _isInitialized; }
  
  virtual void WriteData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData>& data) = 0;
  virtual void GenerateMips() = 0;
  
protected:
  TextureDesc _desc;
	bool _gammaCorrected;
  bool _isInitialized;
};
