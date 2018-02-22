#pragma once
#include <functional>
#include <string>
#include <vector>

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
  uint32 Width = 100;
  uint32 Height = 100;
  Vector2i Position = Vector2i::Zero;
  Colour Colour = Colour::Black;
};

class GuiPanel : public GuiElement
{
public:
  GuiPanel(const GuiPanelDesc& desc);
  ~GuiPanel() {}

  void SetPosition(const Vector2i& position);
  void SetDimensions(const Vector2i& dimensions);
  void SetColour(const Colour& colour);
  inline void SetOnMouseEnter(const std::function<void()>& onMouseEnter) { _onMouseEnter = onMouseEnter; }
  inline void SetOnMouseLeave(const std::function<void()>& onMouseLeave) { _onMouseLeave = onMouseLeave; }

  inline Vector2i GetPosition() const { return _position; }
  inline Vector2i GetDimensions() const { return _dimensions; }
  inline Colour GetColour() const { return _colour; }

private:
  void Draw() override;
  void OnMouseEnter();
  void OnMouseLeave();
  void UploadToGpu();
  inline bool MouseOver() const { return _mouseOver; }

private:
  std::string _name;
  std::vector<std::shared_ptr<GuiElement>> _childElements;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;  
  Colour _colour;
  bool _dirty;
  bool _mouseOver;
  std::function<void()> _onMouseEnter;
  std::function<void()> _onMouseLeave;

  friend class GuiSystem;
};
