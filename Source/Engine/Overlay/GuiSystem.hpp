#pragma once
#include <queue>
#include <stack>

#include "../Core/System.hpp"
#include "GuiCaption.hpp"
#include "GuiPanel.hpp"

struct InputEvent;

namespace Rendering
{
class Shader;
}

class GuiSystem : public System<GuiSystem>
{
public:
  ~GuiSystem() {}
  
  std::shared_ptr<GuiPanel> CreatePanel(const GuiPanelDesc& desc);
  std::shared_ptr<GuiCaption> CreateCaption(const GuiCaptionDesc& desc);

  void OnEvent(const InputEvent& event);
  void Draw();
  
protected:
  GuiSystem();
  
private:
  void SortDraws();

private:
  struct GuiElementDrawItem
  {
    std::weak_ptr<GuiElement> Element;
    std::weak_ptr<Rendering::Shader> Shader;
    uint32 Depth;
  };
  
  std::list<std::shared_ptr<GuiElement>> _elements;
  std::list<std::weak_ptr<GuiElement>> _elementDrawQueue;
  
  friend class System<GuiSystem>;
};
