#include "TextOverlayShader.hpp"

#include "../Rendering/ShaderCollection.h"
#include "../Rendering/Texture.hpp"

TextOverlayShader::TextOverlayShader() :
  Shader(Rendering::ShaderCollection::ShaderDirectory + "TextOverlay.shader")
{
}

void TextOverlayShader::Apply()
{
  Bind();
  _fontAtlas->BindToTextureSlot(0);
}