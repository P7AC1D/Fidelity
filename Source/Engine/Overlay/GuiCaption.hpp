#pragma once
#include <string>

#include "../Maths/Colour.hpp"
#include "Font.hpp"
#include "GuiElement.hpp"

namespace Rendering
{
class VertexBuffer;
}

struct GuiCaptionDesc
{
  std::string Name;
  std::string Text;
  uint32 Width;
  Colour FontColour;
  Font Font;
};

class GuiCaption : public GuiElement
{
public:
  GuiCaption(const GuiCaptionDesc& desc);

  void SetText(const std::string& text);
  void SetFont(const Font& font);
  void SetFontColour(const Colour& colour);

  void Draw() override;

private:
  void UploadToGpu();

private:
  std::string _name;
  std::string _text;
  Colour _fontColour;
  Font _font;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;
};