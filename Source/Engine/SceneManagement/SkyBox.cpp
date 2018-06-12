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

  _vertexBuffer->PushVertexAttrib(VertexAttribType::Vec3);
}

void SkyBox::Draw()
{
  _vertexBuffer->Apply();
}
