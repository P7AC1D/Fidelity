#pragma once
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "ImageFormat.hpp"
#include "ImageVolume.hpp"

class ImageData
{
public:
  ImageData(const ImageVolume& volume, ImageFormat format);
  ImageData(uint32 width, uint32 height, uint32 depth, ImageFormat format);
  
  void WriteData(const std::vector<Colour>& pixels);
  void WritePixel(uint32 x, uint32 y, uint32 z, const Colour& colour);
  Colour ReadPixel(uint32 x, uint32 y, uint32 z) const;
  uint32 GetBytesPerPixel() const;
  
  const std::vector<Colour>& GetPixels() const { return _pixels; }
  
  uint32 GetLeft() const { return _volume.Left; }
  uint32 GetRight() const { return _volume.Right; }
  uint32 GetTop() const { return _volume.Top; }
  uint32 GetBottom() const { return _volume.Bottom; }
  uint32 GetBack() const { return _volume.Back; }
  uint32 GetFront() const { return _volume.Front; }
  
  uint32 GetWidth() const { return _volume.GetWidth(); }
  uint32 GetHeight() const { return _volume.GetHeight(); }
  uint32 GetDepth() const { return _volume.GetDepth(); }
  
  ImageFormat GetFormat() const { return _format; }
  
private:
  ImageVolume _volume;
  ImageFormat _format;
  std::vector<Colour> _pixels;
};
