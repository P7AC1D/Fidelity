#pragma once
#include <memory>
#include <string>

class Texture;

class TextureLoader
{
public:
	static std::shared_ptr<Texture> LoadFromFile2D(const std::string& path, bool generateMips = false);
};