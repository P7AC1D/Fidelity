#include "TextureLoader.hpp"

#include "../Image/ImageLoader.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/Texture.hpp"
#include "../Rendering/Renderer.h"

TextureFormat ToTextureFormat(ImageFormat imageFormat)
{
	switch (imageFormat)
	{
		case ImageFormat::R8: return TextureFormat::R8;
		case ImageFormat::RG8: return TextureFormat::RG8;
		case ImageFormat::RGB8: return TextureFormat::RGB8;
		default:
		case ImageFormat::RGBA8: return TextureFormat::RGBA8;
	}
}

std::shared_ptr<Texture> TextureLoader::LoadFromFile2D(const std::string& path, bool generateMips)
{
	try
	{
		auto imageData = ImageLoader::LoadFromFile(path);

		TextureDesc desc;
		desc.Format = ToTextureFormat(imageData->GetFormat());
		desc.Type = TextureType::Texture2D;
		desc.Width = imageData->GetWidth();
		desc.Height = imageData->GetHeight();
		auto texture = Renderer::GetRenderDevice()->CreateTexture(desc);
    texture->WriteData(0, 0, imageData);
    if (generateMips)
    {
      texture->GenerateMips();
    }
    return texture;
	}
	catch (const std::exception& exception)
	{
		throw std::runtime_error("Could not load texture '" + path + "': " + exception.what());
	}	
}