#include "GuiSystem.hpp"
#include "../Rendering/ShaderCollection.h"
#include "../Shaders/GuiPanelShader.hpp"

std::shared_ptr<GuiPanel> GuiSystem::CreatePanel(const GuiPanelDesc& desc)
{
  auto panel = std::make_shared<GuiPanel>(desc);
  _panels.emplace_back(panel);
  return panel;
}

void GuiSystem::Draw()
{
  auto shader = Rendering::ShaderCollection::Get()->GetShader<GuiPanelShader>();
  for (auto panel : _panels)
  {
    shader->SetColour(panel->GetColour());
    shader->Apply();
    panel->Draw();
  }
}

GuiSystem::GuiSystem()
{
}
