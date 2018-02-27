#include "GuiCaption.hpp"

#include <algorithm>

#include "../Rendering/Renderer.h"
#include "../Rendering/Shader.h"
#include "../Rendering/ShaderCollection.h"
#include "../Rendering/VertexBuffer.h"
#include "../Shaders/TextOverlayShader.hpp"
#include "../Utility/AssetManager.h"
#include "../Utility/FntLoader.hpp"
#include "Font.hpp"

GuiCaption::GuiCaption(const GuiCaptionDesc& desc):
  _name(desc.Name),
  _text(desc.Text),
  _fontName(desc.Font),
  _fontColour(desc.FontColour),
  _fontSize(desc.FontSize)
{
  _font = FntLoader::LoadFontFromFile(RESOURCE_PATH + "/Fonts/" + _fontName + ".fnt");
}

void GuiCaption::SetText(const std::string& text)
{
  _text = text;
  SetDirty(true);
}

void GuiCaption::SetFont(const std::string& font)
{
  _fontName = font;
  _font = FntLoader::LoadFontFromFile(RESOURCE_PATH + "/Fonts/" + _fontName + ".fnt");
  SetDirty(true);
}

void GuiCaption::SetFontColour(const Colour& colour)
{
  _fontColour = colour;
  SetDirty(true);
}

void GuiCaption::Draw()
{
  auto shader = std::dynamic_pointer_cast<TextOverlayShader>(GetShader());
  shader->SetFontAtlas(AssetManager::GetTexture("Fonts/" + _font->TextureFileName));
  shader->SetTextColour(_fontColour);
  shader->Apply();
  
  if (IsDirty())
  {
    UploadToGpu();
    SetDirty(false);
  }

  _vertexBuffer->Apply();
  Rendering::Renderer::Get()->Draw(static_cast<uint32>(_text.size() * 6));
}

std::shared_ptr<Rendering::Shader> GuiCaption::GetShader() const
{
  return Rendering::ShaderCollection::Get()->GetShader<TextOverlayShader>();
}

void GuiCaption::UploadToGpu()
{
  if (!_vertexBuffer)
  {
    _vertexBuffer.reset(new Rendering::VertexBuffer);
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
    _vertexBuffer->PushVertexAttrib(Rendering::VertexAttribType::Vec2);
  }
  
  struct Vertex
  {
    Vector2 Position;
    Vector2 TexCoord;
  };

  std::vector<Vertex> vertices;
  vertices.reserve(24 * _text.size());

  BoundingBox boundingBox = GetBounds();
  Vector2I cursorPos(boundingBox.Left, boundingBox.Top);

  auto renderer = Rendering::Renderer::Get();
  float32 viewportWidth = static_cast<float32>(renderer->GetWidth());
  float32 viewportHeight = static_cast<float32>(renderer->GetHeight());

  float32 fontRelativeSize = _fontSize / static_cast<float32>(_font->Size);
  for (auto& textCharacter : _text)
  {
    auto charImage = std::find_if(_font->Characters.begin(), _font->Characters.end(), [&](const Character& character)
    {
      return textCharacter == character.Id;
    });

    float32 posLeft = 2.0f * (cursorPos.X + charImage->XOffset) * fontRelativeSize / viewportWidth - 1.0f;
    float32 posRight = 2.0f * (cursorPos.X + charImage->XOffset + charImage->Width) * fontRelativeSize / viewportWidth - 1.0f;
    float32 posTop = 1.0f - 2.0f * (cursorPos.Y + charImage->YOffset) * fontRelativeSize / viewportHeight;
    float32 posBottom = 1.0f - 2.0f * (cursorPos.Y + charImage->YOffset + charImage->Height) * fontRelativeSize / viewportHeight;

    float32 uvLeft = charImage->XPos / static_cast<float32>(_font->TextureWidth);
    float32 uvRight = (charImage->XPos + charImage->Width) / static_cast<float32>(_font->TextureWidth);
    float32 uvTop = charImage->YPos / static_cast<float32>(_font->TextureHeight);
    float32 uvBottom = (charImage->YPos + charImage->Height) / static_cast<float32>(_font->TextureHeight);

    Vertex topLeft, topRight, bottomLeft, bottomRight;
    topLeft.Position.X = posLeft;
    topLeft.Position.Y = posTop;
    topLeft.TexCoord.X = uvLeft;
    topLeft.TexCoord.Y = uvTop;

    topRight.Position.X = posRight;
    topRight.Position.Y = posTop;
    topRight.TexCoord.X = uvRight;
    topRight.TexCoord.Y = uvTop;

    bottomLeft.Position.X = posLeft;
    bottomLeft.Position.Y = posBottom;
    bottomLeft.TexCoord.X = uvLeft;
    bottomLeft.TexCoord.Y = uvBottom;

    bottomRight.Position.X = posRight;
    bottomRight.Position.Y = posBottom;
    bottomRight.TexCoord.X = uvRight;
    bottomRight.TexCoord.Y = uvBottom;

    vertices.emplace_back(bottomRight);
    vertices.emplace_back(topRight);
    vertices.emplace_back(topLeft);

    vertices.emplace_back(topLeft);
    vertices.emplace_back(bottomLeft);
    vertices.emplace_back(bottomRight);

    cursorPos.X += charImage->XAdvance;
  }
  _vertexBuffer->UploadData(vertices, Rendering::BufferUsage::Static);
}
