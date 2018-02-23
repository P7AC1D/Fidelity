#pragma once
#include <queue>
#include <stack>

#include "../Core/System.hpp"
#include "GuiPanel.hpp"

struct InputEvent;

class GuiSystem : public System<GuiSystem>
{
public:
  ~GuiSystem() {}
  
  std::shared_ptr<GuiPanel> CreatePanel(const GuiPanelDesc& desc);

  void OnEvent(const InputEvent& event);
  void Draw();
  
protected:
  GuiSystem();
  
private:
  void SetupDraw();

private:
  std::list<std::shared_ptr<GuiPanel>> _panels;
  std::queue<std::weak_ptr<GuiPanel>> _panelDrawQueue;
  
  friend class System<GuiSystem>;
};
