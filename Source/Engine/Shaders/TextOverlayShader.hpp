#pragma once
#include <memory>

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

  void Apply();

private:
  std::shared_ptr<Rendering::Texture> _fontAtlas;
};