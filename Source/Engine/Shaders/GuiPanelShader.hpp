#pragma once
#include <memory>

#include "../Maths/Colour.hpp"
#include "../Rendering/Shader.h"

namespace Rendering
{
class Texture;
}

class GuiPanelShader : public Rendering::Shader
{
public:
  GuiPanelShader();
  
  inline void SetColour(const Colour& colour) { _colour = colour; }
  inline void SetTexture(std::weak_ptr<Rendering::Texture> texture) { _texture = texture; }
  
  void Apply();
  
private:
  Colour _colour;
  std::weak_ptr<Rendering::Texture> _texture;
};

