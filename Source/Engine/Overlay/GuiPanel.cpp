#include "GuiPanel.hpp"

#include "../Maths/Vector2.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/VertexBuffer.h"
#include "../Rendering/OpenGL.h"
#include "../Shaders/GuiPanelShader.hpp"

GuiPanel::GuiPanel(const GuiPanelDesc& desc):
  _name(desc.Name),
  _colour(desc.Colour)
{
  SetBounds(BoundingBox(desc.Left, desc.Right, desc.Top, desc.Bottom));
}

void GuiPanel::SetColour(const Colour& colour)
{
  _colour = colour;
  SetDirty(true);
}

void GuiPanel::AttachChild(std::weak_ptr<GuiElement> child)
{
  _childElements.emplace_back(child);
}

std::shared_ptr<Rendering::Shader> GuiPanel::GetShader() const
{
  return Rendering::ShaderCollection::Get()->GetShader<GuiPanelShader>();
}

void GuiPanel::Draw()
{
  auto shader = std::dynamic_pointer_cast<GuiPanelShader>(GetShader());
  shader->SetColour(_colour);
  shader->Apply();
  
  if (IsDirty())
  {
    UploadToGpu();
    SetDirty(false);
  }

  _vertexBuffer->Apply();
  Rendering::Renderer::Get()->Draw(6);
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

  BoundingBox bounds = GetBounds();

  float32 left = 2.0f * bounds.Left / viewportWidth - 1.0f;
  float32 right = 2.0f * bounds.Right / viewportWidth - 1.0f;
  float32 top = 1.0f - 2.0f * bounds.Top / viewportHeight;
  float32 bottom = 1.0f - 2.0f * bounds.Bottom / viewportHeight;
  
  Vector2 topLeft(left, top);
  Vector2 topRight(right, top);
  Vector2 bottomLeft(left, bottom);
  Vector2 bottomRight(right, bottom);

  std::vector<Vector2> vertexData;
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topRight);
  vertexData.emplace_back(topLeft);
  vertexData.emplace_back(bottomLeft);
  vertexData.emplace_back(bottomRight);
  vertexData.emplace_back(topLeft);

  _vertexBuffer->UploadData(vertexData, Rendering::BufferUsage::Dynamic);
}
