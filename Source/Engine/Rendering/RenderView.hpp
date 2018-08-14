#pragma once
#include <memory>
#include "../Maths/Frustrum.hpp"
#include "../Maths/Rectangle2I.hpp"

class Camera;
class FrameGraph;
class RenderQueue;

struct RenderViewDesc
{
	Rectangle2I Dimensions;
	std::shared_ptr<FrameGraph> FrameGraph;
};

class RenderView
{
public:
	RenderView(const RenderViewDesc& desc, const std::shared_ptr<Camera>& camera);

	std::shared_ptr<Camera> GetCamera() const { return _camera; }

private:
	RenderViewDesc _desc;
	Frustrum _frustrum;
	std::shared_ptr<Camera> _camera;

	std::shared_ptr<RenderQueue> _opaqueQueue;
};