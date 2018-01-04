#pragma once
#include "../Core/Types.hpp"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

namespace Rendering
{
class ConstantBuffer
{
public:
  ConstantBuffer(int32 sizeBytes);
  ~ConstantBuffer();

  void AllocateMemory(int32 sizeBytes);

  template <typename T>
  void UploadData(const T& data);
  template <typename T>
  void UploadSubData(int32 offsetBytes, const T& data);

private:
  ConstantBuffer(ConstantBuffer&) = delete;
  ConstantBuffer(ConstantBuffer&&) = delete;
  ConstantBuffer& operator=(ConstantBuffer&) = delete;
  ConstantBuffer& operator=(ConstantBuffer&&) = delete;

  inline void Initialize();
  void UploadData(int32 offsetBytes, int32 countBytes, const void* data);

  uint32 _uboId;
  int32 _sizeBytes;
  bool _isInitialized;

  friend class Renderer;
};

template<typename T>
inline void ConstantBuffer::UploadData(const T& data)
{
  UploadData(0, sizeof(T), reinterpret_cast<const void*>(*data));
}

template<typename T>
inline void ConstantBuffer::UploadSubData(int32 offsetBytes, const T& data)
{
  UploadData(offsetBytes, sizeof(data), reinterpret_cast<const void*>(&data));
}
}
