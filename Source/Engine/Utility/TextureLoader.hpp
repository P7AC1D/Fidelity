#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class Texture;

class TextureLoader
{
public:
	static std::shared_ptr<Texture> LoadFromFile2D(const std::string& path, bool generateMips = false);

private:
	static std::unordered_map<std::string, std::shared_ptr<Texture>> _cachedTextures;
};