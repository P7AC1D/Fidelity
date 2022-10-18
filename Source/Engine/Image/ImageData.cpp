#include "ImageData.hpp"

#include <stdexcept>

#include "../Utility/Assert.hpp"

ImageData::ImageData(const ImageVolume &volume, ImageFormat format) : _volume(volume), _format(format)
{
	_pixelData.resize(_volume.getSize() * getBytesPerPixel());
}

ImageData::ImageData(uint32 width, uint32 height, uint32 depth, ImageFormat format) : _format(format)
{
	_volume = ImageVolume(0, width, 0, height, 0, depth);
	_pixelData.resize(_volume.getSize() * getBytesPerPixel());
}

void ImageData::writePixel(uint32 x, uint32 y, uint32 z, const Colour &colour)
{
	uint32 width = _volume.getWidth();
	uint32 height = _volume.getHeight();
	uint32 depth = _volume.getDepth();

	ASSERT_TRUE(x <= width, "X out of bounds");
	ASSERT_TRUE(x <= height, "Y out of bounds");
	ASSERT_TRUE(x <= depth, "Z out of bounds");

	auto bytesPerPixel = getBytesPerPixel();
	switch (_format)
	{
	default:
	case ImageFormat::R8:
		_pixelData[x + y * width + z * height] = static_cast<ubyte>(colour[0]);
		break;
	case ImageFormat::RG8:
		_pixelData[x * bytesPerPixel + y * width + z * height] = static_cast<ubyte>(colour[0]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 1] = static_cast<ubyte>(colour[1]);
		break;
	case ImageFormat::RGB8:
		_pixelData[x * bytesPerPixel + y * width + z * height] = static_cast<ubyte>(colour[0]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 1] = static_cast<ubyte>(colour[1]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 2] = static_cast<ubyte>(colour[2]);
		break;
	case ImageFormat::RGBA8:
		_pixelData[x * bytesPerPixel + y * width + z * height] = static_cast<ubyte>(colour[0]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 1] = static_cast<ubyte>(colour[1]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 2] = static_cast<ubyte>(colour[2]);
		_pixelData[x * bytesPerPixel + y * width + z * height + 3] = static_cast<ubyte>(colour[3]);
		break;
	}
}

Colour ImageData::readPixel(uint32 x, uint32 y, uint32 z) const
{
	uint32 width = _volume.getWidth();
	uint32 height = _volume.getHeight();
	uint32 depth = _volume.getDepth();

	ASSERT_TRUE(x <= width, "X out of bounds");
	ASSERT_TRUE(x <= height, "Y out of bounds");
	ASSERT_TRUE(x <= depth, "Z out of bounds");

	auto bytesPerPixel = getBytesPerPixel();
	switch (_format)
	{
	default:
	case ImageFormat::R8:
		return Colour(_pixelData[x * bytesPerPixel + y * width + z * height]);
	case ImageFormat::RG8:
		return Colour(_pixelData[x * bytesPerPixel + y * width + z * height],
									_pixelData[x * bytesPerPixel + y * width + z * height + 1]);
	case ImageFormat::RGB8:
		return Colour(_pixelData[x * bytesPerPixel + y * width + z * height],
									_pixelData[x * bytesPerPixel + y * width + z * height + 1],
									_pixelData[x * bytesPerPixel + y * width + z * height + 2]);
	case ImageFormat::RGBA8:
		return Colour(_pixelData[x * bytesPerPixel + y * width + z * height],
									_pixelData[x * bytesPerPixel + y * width + z * height + 1],
									_pixelData[x * bytesPerPixel + y * width + z * height + 2],
									_pixelData[x * bytesPerPixel + y * width + z * height + 3]);
	}
}

uint32 ImageData::getBytesPerPixel() const
{
	switch (_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::RG8:
		return 2;
	case ImageFormat::RGB8:
		return 3;
	case ImageFormat::RGBA8:
		return 4;
	default:
		throw std::runtime_error("Unsupported ImageFormat");
	}
}

void ImageData::writeData(ubyte *data)
{
	auto byteLen = _volume.getSize() * getBytesPerPixel();
	writeData(std::vector<ubyte>(data, data + byteLen));
}

void ImageData::writeData(const std::vector<ubyte> &data)
{
	ASSERT_TRUE(data.size() == _pixelData.size(), "Pixel data size mismatch");
	_pixelData = data;
}