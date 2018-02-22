#pragma once
#include <memory>
#include "../Maths/Vector2I.hpp"
#include "IScreenElement.hpp"

class GuiElement : public IScreenElement
{
public:
  GuiElement();
  virtual ~GuiElement();

  bool Intersects(const Vector2i& mousePosition);

protected:
  Vector2i _position;
  Vector2i _dimensions;
};
