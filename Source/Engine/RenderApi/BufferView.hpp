#pragma once
#include "ResourceHandle.hpp"
#include "GpuBuffer.hpp"

struct BufferViewDesc
{
  GpuBuffer *buffer = nullptr;
  uint64 offset = 0;
  uint64 size = 0; // 0 == to end
  // Optional format reinterpretation for texel buffers in modern APIs.
};

class BufferView : public ResourceHandle
{
public:
  explicit BufferView(const BufferViewDesc &desc) : _desc(desc) {}
  virtual ~BufferView() = default;

  const BufferViewDesc &getDesc() const { return _desc; }

  // Return the underlying native handle for this view (may alias the buffer handle)
  void *getNativeHandle() const override
  {
    return _desc.buffer ? _desc.buffer->getNativeHandle() : nullptr;
  }
  bool isValid() const override
  {
    return _desc.buffer != nullptr && _desc.buffer->isValid();
  }

protected:
  BufferViewDesc _desc;
};
