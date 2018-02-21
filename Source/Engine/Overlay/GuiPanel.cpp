#include "GuiPanel.hpp"
#include "../Maths/Vector2.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/VertexBuffer.h"

GuiPanel::GuiPanel(const GuiPanelDesc& desc):
  _name(desc.Name),
  _colour(desc.Colour),
  _dirty(true)
{
  SetPosition(desc.Position);
  SetDimensions(Vector2i(desc.Width, desc.Height));
}

void GuiPanel::SetPosition(const Vector2i& position)
{
  _position = position;
  _dirty = true;
}

void GuiPanel::SetDimensions(const Vector2i& dimensions)
{
  _dimensions = dimensions;
  _dirty = true;
}

void GuiPanel::Draw()
{
  if (_dirty)
  {
    UploadToGpu();
    _dirty = false;
  }

  for (auto childElement : _childElements)
  {
    childElement->Draw();
  }
}

void GuiPanel::UploadToGpu()
{
  if (!_vertexBuffer)
  {
    _vertexBuffer.reset(new Rendering::VertexBuffer);
  }

  auto renderer = Rendering::Renderer::Get();
  Vector2 topLeft(_position);
  Vector2 topRight(_position[0] + _dimensions[0], _position[1]);
  Vector2 bottomLeft(_position[0], _position[1] - _dimensions[1]);
  Vector2 bottomRight(_position[0] + _dimensions[0], _position[1] - _dimensions[1]);

  std::vector<Vector2> vertexData;
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topRight);
  vertexData.emplace_back(topLeft);
  vertexData.emplace_back(bottomLeft);
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topLeft);

  _vertexBuffer->UploadData(vertexData, Rendering::BufferUsage::Dynamic);
}