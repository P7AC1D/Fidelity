#include "GuiPanelShader.hpp"

#include "../Rendering/ShaderCollection.h"

static const byte* ColourUniformName = "u_TextColour";

GuiPanelShader::GuiPanelShader() :
Shader(Rendering::ShaderCollection::ShaderDirectory + "GuiPanel.shader")
{
}

void GuiPanelShader::Apply()
{
  Bind();
  SetVec4(ColourUniformName, _colour.ToVec4());
}

