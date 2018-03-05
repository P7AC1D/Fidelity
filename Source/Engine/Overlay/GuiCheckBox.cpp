#include "GuiCheckBox.hpp"

#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/VertexBuffer.h"
#include "../Shaders/GuiPanelShader.hpp"

GuiCheckBox::GuiCheckBox(const GuiCheckBoxDesc& desc):
  _checked(false)
{
  SetBounds(BoundingBox(desc.Position.X, desc.Position.X + desc.Size, desc.Position.Y, desc.Position.Y + desc.Size));
  SetColour(desc.Colour);
}

std::shared_ptr<Rendering::Shader> GuiCheckBox::GetShader() const
{
  return Rendering::ShaderCollection::Get()->GetShader<GuiPanelShader>();
}

void GuiCheckBox::Draw()
{
  auto shader = std::dynamic_pointer_cast<GuiPanelShader>(GetShader());
  shader->SetColour(GetColour());
  shader->SetTexture(_checked ? _checkedTexture : _uncheckedTexture);
  shader->Apply();

  if (IsDirty())
  {
    UploadToGpu();
    SetDirty(false);
  }

  _vertexBuffer->Apply();
  Rendering::Renderer::Get()->Draw(6);
}

void GuiCheckBox::OnMouseClicked(Button button)
{
  if (button == Button::Button_LMouse)
  {
    _checked = !_checked;
    if (_onToggle)
    {
      _onToggle(_checked);
    }
  }
}