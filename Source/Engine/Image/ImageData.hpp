#pragma once
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "ImageFormat.hpp"
#include "ImageVolume.hpp"

class ImageData
{
public:
  ImageData(const ImageVolume &volume, ImageFormat format);
  ImageData(uint32 width, uint32 height, uint32 depth, ImageFormat format);

  void writePixel(uint32 x, uint32 y, uint32 z, const Colour &colour);
  Colour readPixel(uint32 x, uint32 y, uint32 z) const;
  uint32 getBytesPerPixel() const;

  const std::vector<ubyte> &getPixelData() const { return _pixelData; }

  uint32 getLeft() const { return _volume.Left; }
  uint32 getRight() const { return _volume.Right; }
  uint32 getTop() const { return _volume.Top; }
  uint32 getBottom() const { return _volume.Bottom; }
  uint32 getBack() const { return _volume.Back; }
  uint32 getFront() const { return _volume.Front; }

  uint32 getWidth() const { return _volume.getWidth(); }
  uint32 getHeight() const { return _volume.getHeight(); }
  uint32 getDepth() const { return _volume.getDepth(); }

  ImageFormat getFormat() const { return _format; }

  void writeData(ubyte *data);
  void writeData(const std::vector<ubyte> &data);

private:
  ImageVolume _volume;
  ImageFormat _format;
  std::vector<ubyte> _pixelData;
};
