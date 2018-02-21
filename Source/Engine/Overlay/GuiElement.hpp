#pragma once
#include <memory>
#include "../Maths/Vector2i.hpp"
#include "IScreenElement.hpp"

class GuiElement : public IScreenElement
{
public:
  GuiElement() { }
  virtual ~GuiElement() { }

protected:
  Vector2i _position;
  Vector2i _dimensions;
};