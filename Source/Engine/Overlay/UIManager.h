#pragma once
#include <memory>
#include <vector>

namespace UI
{
class Panel;

class UIManager
{
public:
  UIManager();
  ~UIManager();

  void AttachPanel(std::shared_ptr<Panel> panel);

  std::vector<std::shared_ptr<Panel>> GetPanelCollection() const;

private:
  std::vector<std::shared_ptr<Panel>> _panelCollection;
};
}