#include "GuiPanelShader.hpp"

#include "../Rendering/ShaderCollection.h"
#include "../Rendering/Texture.hpp"

static const byte* IsTexturedUniformName = "u_IsTextured";
static const byte* ColourUniformName = "u_PanelColour";
static const byte* PanelTextureSamplerUniformName = "u_PanelTexture";
static const uint32 PanelTextureSamplerUnit = 0;

GuiPanelShader::GuiPanelShader() :
Shader(Rendering::ShaderCollection::ShaderDirectory + "GuiPanel.shader")
{
}

void GuiPanelShader::Apply()
{
  Bind();
  SetVec4(ColourUniformName, _colour.ToVec4());
  if (_texture.expired())
  {
    SetBool(IsTexturedUniformName, false);
  }
  else
  {
    _texture.lock()->BindToTextureSlot(PanelTextureSamplerUnit);
    SetInt(PanelTextureSamplerUniformName, PanelTextureSamplerUnit);
    SetBool(IsTexturedUniformName, true);
  }
}

