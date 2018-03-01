#pragma once
#include <string>

#include "../Maths/Colour.hpp"
#include "GuiElement.hpp"

struct Font;

namespace Rendering
{
class Texture;
class VertexBuffer;
}

struct GuiCaptionDesc
{
  std::string Name;
  std::string Text;
  uint32 FontSize;
  uint32 MaxWidth = 100;
  Colour FontColour = Colour::Black;
  std::string Font = "GillSansMTCondensed";
};

class GuiCaption : public GuiElement
{
public:
  GuiCaption(const GuiCaptionDesc& desc);

  void SetText(const std::string& text);
  void SetFont(const std::string& font);
  void SetFontColour(const Colour& colour);

  void Draw() override;
  
  std::shared_ptr<Rendering::Shader> GetShader() const override;

private:
  void UploadToGpu();

private:
  std::string _name;
  std::string _text;
  std::string _fontName;
  Colour _fontColour;
  std::shared_ptr<Rendering::VertexBuffer> _vertexBuffer;
  std::shared_ptr<Font> _font;
  uint32 _fontSize;
  std::shared_ptr<Rendering::Texture> _fontAtlas;
};
