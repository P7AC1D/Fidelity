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

void TextOverlay::UpdateText(const std::string& text)
{
  _desc.Text = text;
  _dirty = true;
}

void TextOverlay::Draw()
{
  if (!_vertexBuffer)
  {
    _vertexBuffer.reset(new VertexBuffer);
    _vertexBuffer->PushVertexAttrib(VertexAttribType::Vec2);
    _vertexBuffer->PushVertexAttrib(VertexAttribType::Vec2);
  }

  if (_dirty)
  {
    UploadMeshData();
  }

  _vertexBuffer->Apply();
  Renderer::Draw(static_cast<uint32>(_desc.Text.size() * 6));
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

    Vector2 posTopLeft((cursorPos[0] + iter->XOffset) * _desc.Scale / static_cast<float32>(_windowWidth) - 1.0f,
                       (cursorPos[1] - iter->YOffset)  * _desc.Scale/ static_cast<float32>(_windowHeight) + 1.0f);
    Vector2 posTopRight((cursorPos[0] + iter->XOffset + iter->Width) * _desc.Scale / static_cast<float32>(_windowWidth) - 1.0f,
                        (cursorPos[1] - iter->YOffset) * _desc.Scale / static_cast<float32>(_windowHeight) + 1.0f);
    Vector2 posBottomLeft((cursorPos[0] + iter->XOffset) * _desc.Scale / static_cast<float32>(_windowWidth) - 1.0f,
                          (cursorPos[1] - iter->YOffset - iter->Height) * _desc.Scale / static_cast<float32>(_windowHeight) + 1.0f);
    Vector2 posBottomRight((cursorPos[0] + iter->XOffset + iter->Width) * _desc.Scale / static_cast<float32>(_windowWidth) - 1.0f,
                           (cursorPos[1] - iter->YOffset - iter->Height) * _desc.Scale / static_cast<float32>(_windowHeight) + 1.0f);

    Vector2 uvTopLeft(iter->XPos / static_cast<float32>(_desc.Font->Width),
                      iter->YPos / static_cast<float32>(_desc.Font->Height));
    Vector2 uvTopRight((iter->XPos + iter->Width) / static_cast<float32>(_desc.Font->Width),
                       iter->YPos / static_cast<float32>(_desc.Font->Height));
    Vector2 uvBottomLeft(iter->XPos / static_cast<float32>(_desc.Font->Width),
                        (iter->YPos + iter->Height) / static_cast<float32>(_desc.Font->Height));
    Vector2 uvBottomRight((iter->XPos + iter->Width) / static_cast<float32>(_desc.Font->Width),
                          (iter->YPos + iter->Height) / static_cast<float32>(_desc.Font->Height));

    vertices.push_back(posBottomRight);
    vertices.push_back(uvBottomRight);
    vertices.push_back(posTopRight);
    vertices.push_back(uvTopRight);
    vertices.push_back(posTopLeft);
    vertices.push_back(uvTopLeft);

    vertices.push_back(posTopLeft);
    vertices.push_back(uvTopLeft);
    vertices.push_back(posBottomLeft);
    vertices.push_back(uvBottomLeft);
    vertices.push_back(posBottomRight);
    vertices.push_back(uvBottomRight);

    cursorPos[0] += iter->XAdvance;
  }
  _vertexBuffer->UploadData(vertices, BufferUsage::Static);
}
