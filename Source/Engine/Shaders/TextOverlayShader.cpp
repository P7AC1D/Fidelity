#include "TextOverlayShader.hpp"

#include "../Rendering/ShaderCollection.h"
#include "../Rendering/Texture.hpp"

static const uint32 TextureAtlasTextureSlot = 0;
static const byte* TextColourUniformName = "u_TextColour";
static const byte* FontTextScaleUniformName = "u_FontScale";

TextOverlayShader::TextOverlayShader() :
  Shader(Rendering::ShaderCollection::ShaderDirectory + "TextOverlay.shader")
{
}

void TextOverlayShader::Apply()
{
  Bind();
  _fontAtlas->BindToTextureSlot(TextureAtlasTextureSlot);
  SetVec3(TextColourUniformName, _textuColour.ToVec3());
}
