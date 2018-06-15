#include "ImageData.hpp"

#include "../Utility/Assert.hpp"

ImageData::ImageData(const ImageVolume& volume, ImageFormat format): _volume(volume), _format(format)
{
  _pixels.resize(_volume.GetSize());
}

ImageData::ImageData(uint32 width, uint32 height, uint32 depth, ImageFormat format) : _format(format)
{
  ImageVolume volume;
  volume.Left = 0;
  volume.Right = width;
  volume.Bottom = 0;
  volume.Top = height;
  volume.Back = 0;
  volume.Front = depth;
	_volume = volume;
	_pixels.resize(_volume.GetSize());
}

void ImageData::WriteData(const std::vector<Colour>& pixels)
{
  Assert::ThrowIfTrue(_pixels.size() != pixels.size(), "Source pixel array insufficient");
  _pixels = pixels;
}

void ImageData::WritePixel(uint32 x, uint32 y, uint32 z, const Colour& colour)
{
  uint32 width = _volume.GetWidth();
  uint32 height = _volume.GetHeight();
  uint32 depth = _volume.GetDepth();
  
  Assert::ThrowIfFalse(x <= width, "X out of bounds");
  Assert::ThrowIfFalse(x <= height, "Y out of bounds");
  Assert::ThrowIfFalse(x <= depth, "Z out of bounds");
  
  _pixels[x + y * width + z * height] = colour;
}

Colour ImageData::ReadPixel(uint32 x, uint32 y, uint32 z) const
{
  uint32 width = _volume.GetWidth();
  uint32 height = _volume.GetHeight();
  uint32 depth = _volume.GetDepth();
  
  Assert::ThrowIfFalse(x <= width, "X out of bounds");
  Assert::ThrowIfFalse(x <= height, "Y out of bounds");
  Assert::ThrowIfFalse(x <= depth, "Z out of bounds");
  
  return _pixels[x + y * width + z * height];
}

uint32 ImageData::GetBytesPerPixel() const
{
  switch (_format)
  {
    case ImageFormat::R8: return 1;
    case ImageFormat::RG8: return 2;
    case ImageFormat::RGB8: return 3;
    case ImageFormat::RGBA8: return 4;
    default: throw std::runtime_error("Unsupported ImageFormat");
  }
}
