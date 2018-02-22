#pragma once
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
  std::vector<std::shared_ptr<GuiPanel>> _panels;
  
  friend class System<GuiSystem>;
};
