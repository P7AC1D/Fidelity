#pragma once
#include <memory>

#include "../Core/Types.hpp"

namespace Rendering
{
class Texture;
  
enum FrameBufferTarget: uint32
{
  FBT_Colour0 = 1 << 0,
  FBT_Colour1 = 1 << 1,
  FBT_Colour2 = 1 << 2,
  FBT_Depth = 1  << 3,
};

class FrameBuffer
{
public:
  FrameBuffer(uint32 width, uint32 height, uint32 target);
  ~FrameBuffer();

  std::shared_ptr<Texture> GetColourTexture0() { return _colourTexture0; }
  std::shared_ptr<Texture> GetColourTexture1() { return _colourTexture1; }
  std::shared_ptr<Texture> GetColourTexture2() { return _colourTexture2; }
  std::shared_ptr<Texture> GetDepthTexture() { return _depthTexture; }

  void Bind() const;
  void Unbind() const;

  inline uint32 GetWidth() const { return _width; }
  inline uint32 GetHeight() const { return _height; }
  
private:
  std::shared_ptr<Texture> _colourTexture0;
  std::shared_ptr<Texture> _colourTexture1;
  std::shared_ptr<Texture> _colourTexture2;
  std::shared_ptr<Texture> _depthTexture;
  uint32 _width;
  uint32 _height;
  uint32 _fbo;
};
}
