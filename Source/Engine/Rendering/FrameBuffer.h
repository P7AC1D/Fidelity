#pragma once
#include <memory>

#include "../Core/Types.hpp"

namespace Rendering
{
class Texture;

class FrameBuffer
{
public:
  FrameBuffer();
  ~FrameBuffer();

  void Activate();
  void Deactivate() const;

  void SetColourTexture(std::shared_ptr<Texture> texture);
  void SetDepthTexture(std::shared_ptr<Texture> texture);

  std::shared_ptr<Texture> GetColourTexture() const { return _colourTexture; }
  std::shared_ptr<Texture> GetDepthTexture() const { return _depthTexture; }

private:
  void RetrieveViewportDimensions();

  void Bind() const;
  void Unbind() const;
  bool IsBound() const;  

  uint32 _fboId;
  bool isInitialized;
  int32 _prevViewportWidth;
  int32 _prevViewportHeight;
  std::shared_ptr<Texture> _colourTexture;
  std::shared_ptr<Texture> _depthTexture;

  static int32 CurrentlyBoundFboId;

  friend class Renderer;
};
}