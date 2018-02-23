#pragma once
#include <memory>
#include "../Maths/Vector2I.hpp"
#include "IScreenElement.hpp"

struct BoundingBox
{
  BoundingBox(uint32 left, uint32 right, uint32 top, uint32 bottom);
  BoundingBox operator+(const BoundingBox& rhs);

  bool Intersects(const Vector2i& position);

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

protected:
  inline bool IsDirty() const { return _dirty; }
  inline void SetDirty(bool dirty) { _dirty = dirty; }

private:
  BoundingBox _boundingBox;
  std::weak_ptr<GuiElement> _parent;
  bool _dirty;
  uint32 _depth;
};
