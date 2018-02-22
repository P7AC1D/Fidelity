#include "GuiElement.hpp"

GuiElement::GuiElement()
{
}

GuiElement::~GuiElement()
{
}

bool GuiElement::Intersects(const Vector2i& mousePosition)
{
  return (_position.X <= mousePosition.X && mousePosition.X <= _position.X + _dimensions.X) 
    && (_position.Y <= mousePosition.Y && mousePosition.Y <= _position.Y + _dimensions.Y);
}