#include "TextOverlayShader.hpp"

#include "../Maths/Vector2.hpp"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/Texture.hpp"

static const uint32 TextureAtlasTextureSlot = 0;
static const byte* TextColourUniformName = "u_fontColour";
static const byte* TextAtlasUniformName = "u_fontAtlas";
static const byte* TextScaleUniformName = "u_fontScale";

TextOverlayShader::TextOverlayShader() :
  Shader(Rendering::ShaderCollection::ShaderDirectory + "TextOverlay.shader"),
_fontScale(1.0f)
{
}

void TextOverlayShader::Apply()
{
  Bind();
  _fontAtlas->BindToTextureSlot(TextureAtlasTextureSlot);
  SetVec3(TextColourUniformName, _textuColour.ToVec3());
  SetInt(TextAtlasUniformName, TextureAtlasTextureSlot);
  SetFloat(TextScaleUniformName, _fontScale);
}
