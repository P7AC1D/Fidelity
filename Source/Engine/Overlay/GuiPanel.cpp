#include "GuiPanel.hpp"

#include "../Maths/Vector2.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/VertexBuffer.h"
#include "../Shaders/GuiPanelShader.hpp"

GuiPanel::GuiPanel(const GuiPanelDesc& desc)
{
  SetBounds(BoundingBox(desc.Left, desc.Right, desc.Top, desc.Bottom));
  SetName(desc.Name);
  SetColour(desc.Colour);
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
  shader->SetColour(GetColour());
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
