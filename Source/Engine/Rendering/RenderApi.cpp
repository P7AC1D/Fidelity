#include "RenderApi.hpp"

#include "../Maths/Colour.hpp"
#include "ConstantBuffer.h"
#include "OpenGL.h"
#include "Shader.h"

namespace Rendering {
  void RenderApi::SetClearColour(const Colour& colour)
  {
    GLCall(glClearColor(colour[0], colour[1], colour[2], colour[3]));
  }

  void RenderApi::SetShader(const Shader& shader)
  {
    GLCall(glUseProgram(shader._programId));
  }
}