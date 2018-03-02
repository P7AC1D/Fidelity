#include "GuiSystem.hpp"

#include <algorithm>

#include "../Input/InputHandler.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderCollection.h"

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

std::shared_ptr<GuiCheckBox> GuiSystem::CreateCheckBox(const GuiCheckBoxDesc& desc)
{
  auto textBox = std::make_shared<GuiCheckBox>(desc);
  _elements.push_back(textBox);
  return textBox;
}

void GuiSystem::OnEvent(const InputEvent& event)
{
  auto cursorPosition = event.AxisPos;
  for (auto element : _elements)
  {
    bool doesMouseIntersect = element->GetBounds().Intersects(event.AxisPos);
    if (element->MouseOver())
    {
      if (!doesMouseIntersect)
      {
        element->MouseLeft();
      }
    }
    else
    {
      if (doesMouseIntersect)
      {
        element->MouseEntered();
      }
    }

    if (doesMouseIntersect && IsMouseButtonEvent(event.Button))
    {
      element->MouseClicked(event.Button);
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

bool GuiSystem::IsMouseButtonEvent(Button button) const
{
  switch (button)
  {
    case Button::Button_LMouse:
    case Button::Button_RMouse:
    case Button::Button_Mouse3:
    case Button::Button_Mouse4:
    case Button::Button_Mouse5:
    {
      return true;
    }
  }
  return false;
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
                           bool areDepthsEqual = elementA->GetDepth() == elementB->GetDepth();
                           if (!areDepthsEqual)
                           {
                             return false;
                           }
                           bool areShadersEqual = elementA->GetShader() == elementB->GetShader();
                           if (!areShadersEqual)
                           {
                             return false;
                           }
                           return true;
                         });
}
