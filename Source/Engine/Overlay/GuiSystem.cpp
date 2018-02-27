#include "GuiSystem.hpp"

#include <algorithm>

#include "../Input/InputHandler.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"
#include "../Shaders/GuiPanelShader.hpp"

std::shared_ptr<GuiPanel> GuiSystem::CreatePanel(const GuiPanelDesc& desc)
{
  auto panel = std::make_shared<GuiPanel>(desc);
  _elements.push_back(panel);
  return panel;
}

std::shared_ptr<GuiCaption> GuiSystem::CreateCaption(const GuiCaptionDesc& desc)
{
  auto caption = std::make_shared<GuiCaption>(desc);
  _elements.push_back(caption);
  return caption;
}

void GuiSystem::OnEvent(const InputEvent& event)
{
  auto cursorPosition = event.AxisPos;
  for (auto panel : _elements)
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
  SortDraws();

  auto renderer = Rendering::Renderer::Get();
  renderer->DisableDepthTest();
  renderer->DisableStencilTest();
  renderer->EnableBlend(Rendering::BlendType::SrcAlpha, Rendering::BlendType::OneMinusSrcAlpha);
  
  for (auto iter = _elementDrawQueue.begin(); iter != _elementDrawQueue.end(); iter++)
  {
    if (iter->expired())
    {
      _elementDrawQueue.erase(iter);
    }
    else
    {
      iter->lock()->Draw();
    }
  }
  renderer->DisableBlend();
}

GuiSystem::GuiSystem()
{
}

void GuiSystem::SortDraws()
{
  // This check won't work if we add the option to remove GUI elements.
  if (_elements.size() == _elementDrawQueue.size())
  {
    return;
  }
  
  _elementDrawQueue.clear();
  for (auto element : _elements)
  {
    _elementDrawQueue.push_back(element);
  }
  
  _elementDrawQueue.sort([=](const std::weak_ptr<GuiElement>& a, const std::weak_ptr<GuiElement>& b)
                         {
                           auto elementA = a.lock();
                           auto elementB = b.lock();
                           return elementA->GetDepth() == elementB->GetDepth() && elementA->GetShader() == elementB->GetShader();
                         });
}
