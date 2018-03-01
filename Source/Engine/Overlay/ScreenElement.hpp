#pragma once

class IScreenElement
{
public:
  virtual ~IScreenElement() {}

  virtual void Draw() = 0;
};