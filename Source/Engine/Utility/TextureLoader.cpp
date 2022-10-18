#include "TextureLoader.hpp"

#include <stdexcept>

#include "../Image/ImageLoader.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/Texture.hpp"

std::unordered_map<std::string, std::shared_ptr<Texture>> TextureLoader::_cachedTextures;

TextureFormat toTextureFormat(ImageFormat imageFormat)
{
	switch (imageFormat)
	{
	case ImageFormat::R8:
		return TextureFormat::R8;
	case ImageFormat::RG8:
		return TextureFormat::RG8;
	case ImageFormat::RGB8:
		return TextureFormat::RGB8;
	default:
	case ImageFormat::RGBA8:
		return TextureFormat::RGBA8;
	}
}

std::shared_ptr<Texture> TextureLoader::loadFromFile2D(std::shared_ptr<RenderDevice> renderDevice, const std::string &path, bool generateMips, bool sRgb)
{
	auto iter = _cachedTextures.find(path);
	if (iter != _cachedTextures.end())
	{
		return iter->second;
	}

	try
	{
		auto imageData = ImageLoader::loadFromFile(path);

		TextureDesc desc;
		desc.Format = toTextureFormat(imageData->getFormat());
		desc.Type = TextureType::Texture2D;
		desc.Width = imageData->getWidth();
		desc.Height = imageData->getHeight();
		auto texture = renderDevice->createTexture(desc, sRgb);
		texture->writeData(0, 0, imageData);
		if (generateMips)
		{
			texture->generateMips();
		}
		_cachedTextures[path] = texture;
		return texture;
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Could not load texture '" + path + "': " + exception.what());
	}
}