#pragma once
#include <memory>
#include <unordered_map>
#include "../Core/Types.hpp"

class Actor;
class Texture;
class Renderable;
class Transform;

class UiInspector
{
public:
	static void Build(const std::shared_ptr<Actor>& actor);

	static void PushTextureToCache(uint64 ptr, const std::shared_ptr<Texture>& texture);
	static std::shared_ptr<Texture> GetTextureFromCache(uint64 ptr);
	static void ClearCache();

private:
	static void BuildTransform(const std::shared_ptr<Transform>& transform);
	static void BuildRenderable(const std::shared_ptr<Renderable>& renderable);

private:
	static std::unordered_map<uint64, std::shared_ptr<Texture>> _textureCache;
};