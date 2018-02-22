#include "GuiSystem.hpp"

#include "../Input/InputHandler.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Shaders/GuiPanelShader.hpp"

std::shared_ptr<GuiPanel> GuiSystem::CreatePanel(const GuiPanelDesc& desc)
{
  auto panel = std::make_shared<GuiPanel>(desc);
  _panels.emplace_back(panel);
  return panel;
}

void GuiSystem::OnEvent(const InputEvent& event)
{
  auto cursorPosition = event.AxisPos;
  for (auto panel : _panels)
  {
    if (panel->MouseOver())
    {
      if (!panel->Intersects(cursorPosition))
      {
        panel->OnMouseLeave();
      }
    }
    else
    {
      if (panel->Intersects(cursorPosition))
      {
        panel->OnMouseEnter();
      }
    }
  }
}

void GuiSystem::Draw()
{
  auto renderer = Rendering::Renderer::Get();
  renderer->EnableBlend(Rendering::BlendType::SrcAlpha, Rendering::BlendType::OneMinusSrcAlpha);

  auto shader = Rendering::ShaderCollection::Get()->GetShader<GuiPanelShader>();
  for (auto panel : _panels)
  {
    shader->SetColour(panel->GetColour());
    shader->Apply();
    panel->Draw();
  }

  renderer->DisableBlend();
}

GuiSystem::GuiSystem()
{
}
