#pragma once
#include <functional>
#include <list>
#include <string>

#include "../Maths/Colour.hpp"
#include "../Maths/Vector2.hpp"
#include "GuiElement.hpp"

struct InputEvent;

namespace Rendering
{
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

  void SetColour(const Colour& colour);
  inline void SetOnMouseEnter(const std::function<void()>& onMouseEnter) { _onMouseEnter = onMouseEnter; }
  inline void SetOnMouseLeave(const std::function<void()>& onMouseLeave) { _onMouseLeave = onMouseLeave; }

  inline Colour GetColour() const { return _colour; }

  void AttachChild(std::weak_ptr<GuiElement> child);

private:
  void Draw() override;
  void OnMouseEnter();
  void OnMouseLeave();
  void UploadToGpu();
  inline bool MouseOver() const { return _mouseOver; }

private:
  std::string _name;
  std::list<std::weak_ptr<GuiElement>> _childElements;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;  
  Colour _colour;
  bool _mouseOver;
  std::function<void()> _onMouseEnter;
  std::function<void()> _onMouseLeave;

  friend class GuiSystem;
};
