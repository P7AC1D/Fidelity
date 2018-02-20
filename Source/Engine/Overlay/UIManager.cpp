#include "UiManager.h"

namespace UI
{
UIManager::UIManager()
{}

UIManager::~UIManager()
{}

void UIManager::AttachPanel(std::shared_ptr<Panel> panel)
{
  _panelCollection.push_back(panel);
}

std::vector<std::shared_ptr<Panel>> UIManager::GetPanelCollection() const
{
  return _panelCollection;
}
}