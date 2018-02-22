#include "GuiPanel.hpp"
#include "../Maths/Vector2.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/VertexBuffer.h"
#include "../Rendering/OpenGL.h"

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

  _vertexBuffer->Apply();
  Rendering::Renderer::Get()->Draw(6);

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
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
  }
  
  auto renderer = Rendering::Renderer::Get();
  auto viewportWidth = static_cast<float32>(renderer->GetWidth());
  auto viewportHeight = static_cast<float32>(renderer->GetHeight());
  
  Vector2 topLeft(_position[0] / viewportWidth, _position[1]/ viewportHeight);
  Vector2 topRight((_position[0] + _dimensions[0]) / viewportWidth, _position[1] / viewportHeight);
  Vector2 bottomLeft((_position[0]) / viewportWidth, (_position[1] - _dimensions[1]) / viewportHeight);
  Vector2 bottomRight((_position[0] + _dimensions[0]) / viewportWidth, (_position[1] - _dimensions[1]) / viewportHeight);

  topLeft[0] -= 1.0f;
  topRight[0] -= 1.0f;
  bottomLeft[0] -= 1.0f;
  bottomRight[0] -= 1.0f;

  topLeft[1] += 1.0f;
  topRight[1] += 1.0f;
  bottomLeft[1] += 1.0f;
  bottomRight[1] += 1.0f;

  std::vector<Vector2> vertexData;
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topRight);
  vertexData.emplace_back(topLeft);
  vertexData.emplace_back(bottomLeft);
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topLeft);

  _vertexBuffer->UploadData(vertexData, Rendering::BufferUsage::Dynamic);
}
