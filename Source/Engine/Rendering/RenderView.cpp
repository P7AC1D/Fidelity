#include "RenderView.hpp"

#include "../SceneManagement/Camera.hpp"

RenderView::RenderView(const RenderViewDesc& desc, const std::shared_ptr<Camera>& camera) : _desc(desc), _camera(camera), _frustrum(camera->GetProjection())
{
}
