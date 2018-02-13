#pragma once
#include "../Core/Types.hpp"
#include "TextureCommon.hpp"

namespace Rendering
{
enum class TextureCubeFace
{
  Right,
  Left,
  Top,
  Bottom,
  Back,
  Front
};

struct TextureCubeDesc
{
  uint32 Width;
  uint32 Height;
  PixelFormat Format = PixelFormat::RGB8;
  WrapMethod WrapMethod = WrapMethod::ClampToBorder;
  MinFilter MinFilter = MinFilter::Nearest;
  MagFilter MagFiler = MagFilter::Nearest;
};

class TextureCube
{
public:
  TextureCube(TextureCubeDesc desc);
  ~TextureCube();

  inline TextureCubeDesc GetDesc() const { return _desc; }

  void UploadData(TextureCubeFace face, const ubyte* data, uint32 level = 0);
  
  void SetWrapMethod(WrapMethod wrapMethod);
  void SetMinFilter(MinFilter minFilter);
  void SetMagFilter(MagFilter magFilter);

  void GenerateMipMaps();
  void BindToTextureSlot(uint32 slot);

private:
  void Bind();
  void Unbind();

private:
  uint32 _id;
  TextureCubeDesc _desc;
};
}