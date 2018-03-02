#pragma once
#include <list>
#include <string>

#include "../Maths/Colour.hpp"
#include "../Maths/Vector2.hpp"
#include "GuiElement.hpp"

struct InputEvent;

namespace Rendering
{
class Texture;
class VertexBuffer;
}

struct GuiPanelDesc
{
  std::string Name;
  uint32 Left = 0;
  uint32 Right = 100;
  uint32 Bottom = 100;
  uint32 Top = 0;
  Vector2I Position = Vector2I::Zero;
  Colour Colour = Colour::Black;
};

class GuiPanel : public GuiElement
{
public:
  GuiPanel(const GuiPanelDesc& desc);
  ~GuiPanel() {}

  void SetTexture(std::shared_ptr<Rendering::Texture> texture);

  void AttachChild(std::weak_ptr<GuiElement> child);
  
  std::shared_ptr<Rendering::Shader> GetShader() const override;  

private:
  void Draw() override final;
  void OnMouseClicked(Button button) override final {}

private:
  std::list<std::weak_ptr<GuiElement>> _childElements;
  std::shared_ptr<Rendering::Texture> _texture;
};
