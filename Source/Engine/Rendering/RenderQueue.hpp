#pragma once
#include "../Core/Types.hpp"
#include "../Maths/AABB.hpp"
#include <memory>
#include <set>

class Camera;
class Renderable;
class Transform;

struct RenderableItem
{
	RenderableItem(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform, const Aabb& bounds) :
		Renderable(renderable),
		Transform(transform),
		Bounds(bounds)
	{}

	std::shared_ptr<Renderable> Renderable;
	std::shared_ptr<Transform> Transform;
	Aabb Bounds;
};

class RenderQueue
{
public:
	void Push(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform, const Aabb& Bounds);
	void Clear();

	std::set<RenderableItem>::const_iterator GetIteratorBegin();
	std::set<RenderableItem>::const_iterator GetIteratorEnd();

private:
	struct RenderableItemCompare
	{
		bool operator()(const RenderableItem& lhs, const RenderableItem& rhs) const;
	};

private:
	std::shared_ptr<Camera> _activeCamera;
	std::set<RenderableItem, RenderableItemCompare> _queue;
};