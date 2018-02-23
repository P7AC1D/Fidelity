#include "GuiElement.hpp"

#include <algorithm>
#include <cassert>

BoundingBox::BoundingBox(uint32 left, uint32 right, uint32 top, uint32 bottom) :
  Left(left),
  Right(right),
  Top(top),
  Bottom(bottom)
{
}

BoundingBox BoundingBox::operator+(const BoundingBox& rhs)
{
  Left += rhs.Left;
  Right += rhs.Right;
  Top += rhs.Top;
  Bottom += rhs.Bottom;
  return BoundingBox(Left, Right, Top, Bottom);
}

bool BoundingBox::Intersects(const Vector2i& position)
{
  return position.X >= Left && position.X <= Right && position.Y >= Top && position.Y <= Bottom;
}

GuiElement::GuiElement():
  _boundingBox(0, 100, 0, 100),
  _dirty(true),
  _depth(0)
{
}

GuiElement::~GuiElement()
{
}

void GuiElement::SetParent(std::weak_ptr<GuiElement> parent)
{
  _parent = parent;
  SetBounds(_boundingBox);
  _depth++;
}

void GuiElement::SetBounds(const BoundingBox& bounds)
{
  assert(bounds.Left < bounds.Right);
  assert(bounds.Top < bounds.Bottom);

  if (_parent.expired())
  {
    _boundingBox = bounds;
  }
  else
  {
    std::shared_ptr<GuiElement> parent = _parent.lock();
    BoundingBox parentBounds = parent->GetBounds();
    _boundingBox.Left = std::min(parentBounds.Left + bounds.Left, parentBounds.Right);
    _boundingBox.Top = std::min(parentBounds.Top + bounds.Top, parentBounds.Bottom);
    _boundingBox.Right = std::min(parentBounds.Left + bounds.Right, parentBounds.Right);
    _boundingBox.Bottom = std::min(parentBounds.Top + bounds.Bottom, parentBounds.Bottom);
  }
  SetDirty(true);
}