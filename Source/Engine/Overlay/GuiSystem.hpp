#pragma once
#include <queue>
#include <stack>

#include "../Core/System.hpp"
#include "../Input/InputBindings.hpp"
#include "GuiCaption.hpp"
#include "GuiCheckBox.hpp"
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
  std::shared_ptr<GuiCheckBox> CreateCheckBox(const GuiCheckBoxDesc& desc);

  void OnEvent(const InputEvent& event);
  void Draw();
  
protected:
  GuiSystem();
  
private:
  bool IsMouseButtonEvent(Button button) const;
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
