#pragma once
#include <memory>
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Vector2.hpp"

namespace Rendering
{
class Renderer;
class Texture;
}

namespace UI
{
struct PanelBounds
{
  Vector2 TopLeft;
  Vector2 TopRight;
  Vector2 BottomLeft;
  Vector2 BottomRight;
};

class Panel
{
public:
  Panel(std::string name, uint32 width, uint32 height, uint32 xPos, uint32 yPos);
  ~Panel();

  void SetTexture(std::shared_ptr<Rendering::Texture> texture);

private:
  friend class Rendering::Renderer;

  uint32 _width;
  uint32 _height;
  uint32 _xPos;
  uint32 _yPos;
  std::string _name;
  std::shared_ptr<Rendering::Texture> _texture;
  PanelBounds _panelBounds;
};
}