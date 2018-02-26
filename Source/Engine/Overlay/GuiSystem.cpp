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
      if (!panel->GetBounds().Intersects(cursorPosition))
      {
        panel->OnMouseLeave();
      }
    }
    else
    {
      if (panel->GetBounds().Intersects(cursorPosition))
      {
        panel->OnMouseEnter();
      }
    }
  }
}

void GuiSystem::Draw()
{
  SetupDraw();

  auto renderer = Rendering::Renderer::Get();
  renderer->EnableBlend(Rendering::BlendType::SrcAlpha, Rendering::BlendType::OneMinusSrcAlpha);
  auto shader = Rendering::ShaderCollection::Get()->GetShader<GuiPanelShader>();

  while (!_panelDrawQueue.empty())
  {
    auto queuedItem = _panelDrawQueue.front();
    if (!queuedItem.expired())
    {
      auto panel = queuedItem.lock();
      shader->SetColour(panel->GetColour());
      shader->Apply();
      panel->Draw();
    }    
    _panelDrawQueue.pop();
  }

  renderer->DisableBlend();
}

GuiSystem::GuiSystem()
{
}

void GuiSystem::SetupDraw()
{
  uint32 currentDepth(0);
  std::list<std::shared_ptr<GuiPanel>> panels(_panels);
  do
  {    
    for (auto iter = panels.begin(); iter != panels.end();)
    {
      if ((*iter)->GetDepth() == currentDepth)
      {
        _panelDrawQueue.emplace((*iter));
        iter = panels.erase(iter);
      }
      else
      {
        iter++;
      }
    }
    currentDepth++;
  }
  while (!panels.empty());
}

void GuiSystem::SortDraws()
{

}