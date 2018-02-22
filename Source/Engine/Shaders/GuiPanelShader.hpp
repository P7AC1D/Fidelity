#pragma once
#include <memory>

#include "../Maths/Colour.hpp"
#include "../Rendering/Shader.h"

class GuiPanelShader : public Rendering::Shader
{
public:
  GuiPanelShader();
  
  inline void SetColour(const Colour& colour) { _colour = colour; }
  
  void Apply();
  
private:
  Colour _colour;
};

