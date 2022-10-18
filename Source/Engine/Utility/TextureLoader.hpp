#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class Texture;
class RenderDevice;

class TextureLoader
{
public:
	static std::shared_ptr<Texture> loadFromFile2D(std::shared_ptr<RenderDevice> renderDevice, const std::string &path, bool generateMips = false, bool sRgb = false);

private:
	static std::unordered_map<std::string, std::shared_ptr<Texture>> _cachedTextures;
};