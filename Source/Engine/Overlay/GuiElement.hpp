#pragma once
#include <functional>
#include <memory>
#include <string>

#include "../Input/InputBindings.hpp"
#include "../Maths/Colour.hpp"
#include "../Maths/Vector2I.hpp"
#include "ScreenElement.hpp"

namespace Rendering
{
class Shader;
class VertexBuffer;
}

struct BoundingBox
{
  BoundingBox();
  BoundingBox(uint32 left, uint32 right, uint32 top, uint32 bottom);
  BoundingBox operator+(const BoundingBox& rhs);

  bool Intersects(const Vector2I& position) const;

  uint32 Left;
  uint32 Right;
  uint32 Top;
  uint32 Bottom;
};

class GuiElement : public IScreenElement
{
public:
  GuiElement();
  virtual ~GuiElement();

  void SetParent(std::weak_ptr<GuiElement> parent);
  void SetBounds(const BoundingBox& bounds);
  inline void SetName(const std::string& name) { _name = name; }

  inline const Colour& GetColour() const { return _colour; }
  inline const BoundingBox& GetBounds() const { return _boundingBox; }
  inline const std::string& GetName() const { return _name; }
  inline uint32 GetDepth() const { return _depth; }

  inline void SetColour(const Colour& colour) { _colour = colour; }  
  inline void SetMouseEnteredCallback(const std::function<void()>& onMouseEnter) { _onMouseEnter = onMouseEnter; }
  inline void SetMouseLeftCallback(const std::function<void()>& onMouseLeave) { _onMouseLeave = onMouseLeave; }
  inline void SetMouseClickCallback(const std::function<void(Button button)>& onMouseClick) { _onMouseClick = onMouseClick; }
  
  virtual std::shared_ptr<Rendering::Shader> GetShader() const = 0;

protected:
  inline bool IsDirty() const { return _dirty; }
  inline void SetDirty(bool dirty) { _dirty = dirty; }

  virtual void UploadToGpu();
  
  inline bool MouseOver() const { return _mouseOver; }

  void MouseEntered();
  void MouseLeft();
  void MouseClicked(Button button);

  virtual void OnMouseClicked(Button button) = 0;

protected:
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;

private:
  BoundingBox _boundingBox;
  std::weak_ptr<GuiElement> _parent;
  bool _dirty;
  bool _mouseOver;
  uint32 _depth;
  Colour _colour;
  std::string _name;
  std::function<void()> _onMouseEnter;
  std::function<void()> _onMouseLeave;
  std::function<void(Button button)> _onMouseClick;
  
  friend class GuiSystem;
};
