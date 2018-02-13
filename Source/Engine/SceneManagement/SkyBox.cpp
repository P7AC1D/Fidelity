#include "SkyBox.hpp"

#include <vector>

#include "../Geometry/Cube.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/TextureCube.hpp"
#include "../Rendering/VertexBuffer.h"
#include "../Rendering/OpenGL.h"

using namespace Rendering;

SkyBox::SkyBox()
{
  _vertexBuffer.reset(new VertexBuffer);

  Cube cube;
  _vertexBuffer->UploadData(cube.GetPositions(), BufferUsage::Static);

  _vertexBuffer->Bind();
  GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  GLCall(glEnableVertexAttribArray(0));
  _vertexBuffer->Unbind();
}

void SkyBox::Draw()
{
  _vertexBuffer->Bind();
  Renderer::Draw(36);
}