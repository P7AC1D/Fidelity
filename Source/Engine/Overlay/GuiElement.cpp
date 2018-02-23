#include "GuiElement.hpp"

#include <algorithm>
#include <cassert>

#include "../Rendering/Renderer.h"

BoundingBox::BoundingBox():
  Left(0),
  Right(100),
  Top(0),
  Bottom(100)
{
}

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

bool BoundingBox::Intersects(const Vector2I& position)
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

  BoundingBox parentBounds;
  if (_parent.expired())
  {
    auto renderer = Rendering::Renderer::Get();
    parentBounds.Left = 0;
    parentBounds.Right = renderer->GetWidth();
    parentBounds.Top = 0;
    parentBounds.Bottom = renderer->GetHeight();
  }
  else
  {
    std::shared_ptr<GuiElement> parent = _parent.lock();
    parentBounds = parent->GetBounds();
  }

  _boundingBox.Left = std::min(parentBounds.Left + bounds.Left, parentBounds.Right);
  _boundingBox.Top = std::min(parentBounds.Top + bounds.Top, parentBounds.Bottom);
  _boundingBox.Right = std::min(parentBounds.Left + bounds.Right, parentBounds.Right);
  _boundingBox.Bottom = std::min(parentBounds.Top + bounds.Bottom, parentBounds.Bottom);
  SetDirty(true);
}