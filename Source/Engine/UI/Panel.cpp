#include "Panel.h"

#include "../Rendering/Texture.h"
#include "../Utility/AssetManager.h"

using namespace Rendering;
using namespace Utility;

namespace UI
{
Panel::Panel(std::string name, uint32 width, uint32 height, uint32 xPos, uint32 yPos) :
  _width(width),
  _height(height),
  _xPos(xPos),
  _yPos(yPos),
  _name(std::move(name))
{
  _panelBounds.TopLeft = Vector2(xPos, yPos);
  _panelBounds.TopRight = Vector2(xPos + width, yPos);
  _panelBounds.BottomLeft = Vector2(xPos, yPos + height);
  _panelBounds.BottomRight = Vector2(xPos + width, yPos + height);
}

Panel::~Panel()
{}

void Panel::SetTexture(std::shared_ptr<Texture> texture)
{
  _texture = texture;
}
}