#pragma once
#include <functional>
#include <memory>

#include "../Maths/Vector2I.hpp"
#include "IScreenElement.hpp"

namespace Rendering
{
class Shader;
}

struct BoundingBox
{
  BoundingBox();
  BoundingBox(uint32 left, uint32 right, uint32 top, uint32 bottom);
  BoundingBox operator+(const BoundingBox& rhs);

  bool Intersects(const Vector2I& position);

  uint32 Left;
  uint32 Right;
  uint32 Top;
  uint32 Bottom;
};

class GuiElement : public IScreenElement, public std::enable_shared_from_this<GuiElement>
{
public:
  GuiElement();
  virtual ~GuiElement();

  void SetParent(std::weak_ptr<GuiElement> parent);
  void SetBounds(const BoundingBox& bounds);

  inline BoundingBox GetBounds() const { return _boundingBox; }
  inline uint32 GetDepth() const { return _depth; }
  
  inline void SetOnMouseEnter(const std::function<void()>& onMouseEnter) { _onMouseEnter = onMouseEnter; }
  inline void SetOnMouseLeave(const std::function<void()>& onMouseLeave) { _onMouseLeave = onMouseLeave; }
  
  virtual std::shared_ptr<Rendering::Shader> GetShader() const = 0;

protected:
  inline bool IsDirty() const { return _dirty; }
  inline void SetDirty(bool dirty) { _dirty = dirty; }
  
  inline bool MouseOver() const { return _mouseOver; }
  void OnMouseEnter();
  void OnMouseLeave();

private:
  BoundingBox _boundingBox;
  std::weak_ptr<GuiElement> _parent;
  bool _dirty;
  bool _mouseOver;
  uint32 _depth;
  std::function<void()> _onMouseEnter;
  std::function<void()> _onMouseLeave;
  
  friend class GuiSystem;
};
