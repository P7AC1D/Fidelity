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

void GuiPanel::SetTexture(std::shared_ptr<Rendering::Texture> texture)
{
  _texture = texture;
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
  shader->SetTexture(_texture);
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
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
  }

  struct Vertex
  {
    Vector2 Position;
    Vector2 TexCoord;
  };
  
  auto renderer = Rendering::Renderer::Get();
  auto viewportWidth = static_cast<float32>(renderer->GetWidth());
  auto viewportHeight = static_cast<float32>(renderer->GetHeight());

  BoundingBox bounds = GetBounds();

  float32 posLeft = 2.0f * bounds.Left / viewportWidth - 1.0f;
  float32 posRight = 2.0f * bounds.Right / viewportWidth - 1.0f;
  float32 posTop = 1.0f - 2.0f * bounds.Top / viewportHeight;
  float32 posBottom = 1.0f - 2.0f * bounds.Bottom / viewportHeight;

  Vertex topLeft, topRight, bottomLeft, bottomRight;
  topLeft.Position.X = posLeft;
  topLeft.Position.Y = posTop;
  topLeft.TexCoord.X = 0.0f;
  topLeft.TexCoord.Y = 1.0f;

  topRight.Position.X = posRight;
  topRight.Position.Y = posTop;
  topRight.TexCoord.X = 1.0f;
  topRight.TexCoord.Y = 1.0f;

  bottomLeft.Position.X = posLeft;
  bottomLeft.Position.Y = posBottom;
  bottomLeft.TexCoord.X = 0.0f;
  bottomLeft.TexCoord.Y = 0.0f;

  bottomRight.Position.X = posRight;
  bottomRight.Position.Y = posBottom;
  bottomRight.TexCoord.X = 1.0f;
  bottomRight.TexCoord.Y = 0.0f;

  std::vector<Vertex> vertexData;
  vertexData.push_back(bottomRight);
  vertexData.push_back(topRight);
  vertexData.push_back(topLeft);
  vertexData.push_back(bottomLeft);
  vertexData.push_back(bottomRight);
  vertexData.push_back(topLeft);

  _vertexBuffer->UploadData(vertexData, Rendering::BufferUsage::Dynamic);
}
