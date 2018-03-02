#include "GuiElement.hpp"

#include <algorithm>
#include <cassert>

#include "../Rendering/Renderer.h"
#include "../Rendering/VertexBuffer.h"

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

bool BoundingBox::Intersects(const Vector2I& position) const
{
  return position.X >= Left && position.X <= Right && position.Y >= Top && position.Y <= Bottom;
}

GuiElement::GuiElement():
  _boundingBox(0, 100, 0, 100),
  _dirty(true),
  _mouseOver(false),
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

void GuiElement::UploadToGpu()
{
  if (!_vertexBuffer)
  {
    _vertexBuffer.reset(new Rendering::VertexBuffer);
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
  }

  struct Vertex
  {
    Vector2 Position;
    Vector2 TexCoord;
  };
  
  auto renderer = Rendering::Renderer::Get();
  auto viewportWidth = static_cast<float32>(renderer->GetWidth());
  auto viewportHeight = static_cast<float32>(renderer->GetHeight());

  BoundingBox bounds = GetBounds();

  float32 posLeft = 2.0f * bounds.Left / viewportWidth - 1.0f;
  float32 posRight = 2.0f * bounds.Right / viewportWidth - 1.0f;
  float32 posTop = 1.0f - 2.0f * bounds.Top / viewportHeight;
  float32 posBottom = 1.0f - 2.0f * bounds.Bottom / viewportHeight;

  Vertex topLeft, topRight, bottomLeft, bottomRight;
  topLeft.Position.X = posLeft;
  topLeft.Position.Y = posTop;
  topLeft.TexCoord.X = 0.0f;
  topLeft.TexCoord.Y = 1.0f;

  topRight.Position.X = posRight;
  topRight.Position.Y = posTop;
  topRight.TexCoord.X = 1.0f;
  topRight.TexCoord.Y = 1.0f;

  bottomLeft.Position.X = posLeft;
  bottomLeft.Position.Y = posBottom;
  bottomLeft.TexCoord.X = 0.0f;
  bottomLeft.TexCoord.Y = 0.0f;

  bottomRight.Position.X = posRight;
  bottomRight.Position.Y = posBottom;
  bottomRight.TexCoord.X = 1.0f;
  bottomRight.TexCoord.Y = 0.0f;

  std::vector<Vertex> vertexData;
  vertexData.push_back(bottomRight);
  vertexData.push_back(topRight);
  vertexData.push_back(topLeft);
  vertexData.push_back(bottomLeft);
  vertexData.push_back(bottomRight);
  vertexData.push_back(topLeft);

  _vertexBuffer->UploadData(vertexData, Rendering::BufferUsage::Static);
}

void GuiElement::MouseEntered()
{
  _mouseOver = true;
  if (_onMouseEnter)
  {
    _onMouseEnter();
  }
}

void GuiElement::MouseLeft()
{
  _mouseOver = false;
  if (_onMouseLeave)
  {
    _onMouseLeave();
  }
}

void GuiElement::MouseClicked(Button button)
{
  if (_onMouseClick)
  {
    _onMouseClick(button);
  }
  OnMouseClicked(button);
}