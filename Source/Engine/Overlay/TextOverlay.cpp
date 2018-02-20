#include "TextOverlay.hpp"

#include <algorithm>

#include "../Maths/Vector2.hpp"
#include "../Rendering/VertexBuffer.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/OpenGL.h"

using namespace Rendering;

TextOverlay::TextOverlay(const TextOverlayDesc& desc, uint32 windowWidth, uint32 windowHeight):
  _dirty(true),
  _windowWidth(windowWidth),
  _windowHeight(windowHeight),
  _desc(desc)
{
}

void TextOverlay::Draw()
{
  if (!_vertexBuffer)
  {
    _vertexBuffer.reset(new VertexBuffer);
  }

  if (_dirty)
  {
    UploadMeshData();
  }

  _vertexBuffer->Bind();
  Renderer::Draw(_desc.Text.size() * 6);
}

void TextOverlay::UploadMeshData()
{
  _dirty = false;
  std::vector<Vector2> vertices;
  vertices.reserve(24 * _desc.Text.size());
  Vector2i cursorPos(_desc.Position);

  for (auto& textCharacter : _desc.Text)
  {
    auto iter = std::find_if(_desc.Font->Characters.begin(), _desc.Font->Characters.end(), [&](const Character& character)
    {
      return textCharacter == character.Id;
    });

    if (iter == _desc.Font->Characters.end())
    {
      throw std::runtime_error("Unsupported character.");
    }

    Vector2 topLeft((cursorPos[0] + iter->XOffset) / static_cast<float32>(_windowWidth),
                    (cursorPos[1] - iter->YOffset) / static_cast<float32>(_windowHeight));
    Vector2 topRight((cursorPos[0] + iter->XOffset + iter->Width) / static_cast<float32>(_windowWidth),
                     (cursorPos[1] - iter->YOffset) / static_cast<float32>(_windowHeight));
    Vector2 bottomLeft((cursorPos[0] + iter->XOffset) / static_cast<float32>(_windowWidth),
                       (cursorPos[1] - iter->YOffset - iter->Height) / static_cast<float32>(_windowHeight));
    Vector2 bottomRight((cursorPos[0] + iter->XOffset + iter->Width) / static_cast<float32>(_windowWidth),
                        (cursorPos[1] - iter->YOffset - iter->Height) / static_cast<float32>(_windowHeight));

    Vector2 uvTopLeft(iter->XPos / static_cast<float32>(_desc.Font->Width),
                      iter->YPos / static_cast<float32>(_desc.Font->Height));
    Vector2 uvTopRight((iter->XPos + iter->Width) / static_cast<float32>(_desc.Font->Width),
                       iter->YPos / static_cast<float32>(_desc.Font->Height));
    Vector2 uvBottomLeft(iter->XPos / static_cast<float32>(_desc.Font->Width),
                        (iter->YPos + iter->Height) / static_cast<float32>(_desc.Font->Height));
    Vector2 uvBottomRight((iter->XPos + iter->Width) / static_cast<float32>(_desc.Font->Width),
                          (iter->YPos + iter->Height) / static_cast<float32>(_desc.Font->Height));

    vertices.push_back(bottomRight);
    vertices.push_back(uvBottomRight);
    vertices.push_back(topRight);
    vertices.push_back(uvTopRight);
    vertices.push_back(topLeft);
    vertices.push_back(uvTopLeft);

    vertices.push_back(topLeft);
    vertices.push_back(uvTopLeft);
    vertices.push_back(bottomLeft);
    vertices.push_back(uvBottomLeft);
    vertices.push_back(bottomRight);
    vertices.push_back(uvBottomRight);

    cursorPos[0] += iter->XAdvance;
  }
  _vertexBuffer->UploadData(vertices, BufferUsage::Static);

  // TODO: This needs to be moved into the VertexBuffer object
  _vertexBuffer->Bind();
  GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, nullptr));
  GLCall(glEnableVertexAttribArray(0));
  GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (void*)8));
  GLCall(glEnableVertexAttribArray(1));
  _vertexBuffer->Unbind();
}