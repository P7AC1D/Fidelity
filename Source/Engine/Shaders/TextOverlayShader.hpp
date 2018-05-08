#pragma once
#include <memory>

#include "../Maths/Colour.hpp"
#include "../Rendering/Shader.h"

namespace Rendering
{
class Texture;
}

class TextOverlayShader : public Rendering::Shader
{
public:
  TextOverlayShader();

  inline void SetFontAtlas(const std::shared_ptr<Rendering::Texture>& fontAtlas) { _fontAtlas = fontAtlas; }
  inline void SetTextColour(const Colour& colour) { _textuColour = colour; }
  inline void SetTextScale(float32 scale) { _fontScale = scale; }

  void Apply();

private:
  std::shared_ptr<Rendering::Texture> _fontAtlas;
  Colour _textuColour;
  float32 _fontScale;
};
