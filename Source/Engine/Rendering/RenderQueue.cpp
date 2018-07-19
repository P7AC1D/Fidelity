#include "RenderQueue.hpp"

#include "../Rendering/Material.hpp"
#include "../Rendering/StaticMesh.h"
#include "../Rendering/Renderable.hpp"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/Camera.hpp"
#include "../SceneManagement/Transform.h"

void RenderQueue::Push(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform)
{
	_queue.emplace(renderable, transform);
}

void RenderQueue::Clear()
{
	_queue.clear();
}

std::set<RenderableItem>::const_iterator RenderQueue::GetIteratorBegin()
{
	return _queue.begin();
}

std::set<RenderableItem>::const_iterator RenderQueue::GetIteratorEnd()
{
	return _queue.end();
}

bool RenderQueue::RenderableItemCompare::operator()(const RenderableItem& lhs, const RenderableItem& rhs) const
{
	auto camPos = SceneManager::Get()->GetCamera()->GetPosition();

	auto lhsMat = lhs.Renderable->GetMesh()->GetMaterial();
	auto rhsMat = rhs.Renderable->GetMesh()->GetMaterial();

	if (lhsMat->GetId() < rhsMat->GetId())
	{
		return true;
	}

	if (lhsMat->GetId() > rhsMat->GetId())
	{
		return false;
	}
	
	auto lhsPos = lhs.Transform->GetPosition();
	auto rhsPos = rhs.Transform->GetPosition();

	auto distToLhs = Vector4::Length(camPos - Vector3(lhsPos));
	auto distToRhs = Vector4::Length(camPos - Vector3(rhsPos));
	return distToLhs < distToRhs;
}
