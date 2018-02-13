#include "SkyBox.hpp"

#include "../Geometry/Cube.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/TextureCube.hpp"
#include "../Rendering/VertexBuffer.h"

using namespace Rendering;

SkyBox::SkyBox()
{
  Cube cube;
  _vertexBuffer.reset(new VertexBuffer);

  auto positions = cube.GetPositions();
  auto texCoords = cube.GetTexCoords();

  std::vector<float32> dataToUpload;
  dataToUpload.reserve(positions.size() * 3 + texCoords.size() * 2);
  for (size_t i = 0; i < positions.size(); i++)
  {
    dataToUpload.push_back(positions[i][0]);
    dataToUpload.push_back(positions[i][1]);
    dataToUpload.push_back(positions[i][2]);
    dataToUpload.push_back(texCoords[i][0]);
    dataToUpload.push_back(texCoords[i][2]);
  }
  _vertexBuffer->UploadData(dataToUpload, BufferUsage::Static);
}

void SkyBox::Draw()
{
  _vertexBuffer->Bind();
  Renderer::Draw(36);
}