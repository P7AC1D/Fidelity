#pragma once
#include <vector>

#include "../Core/Types.hpp"

namespace Rendering {
  class IndexBuffer
  {
  public:
    IndexBuffer();
    ~IndexBuffer();

    inline uint32 GetId() const { return _ibo; }

    void Bind();

    void UploadData(const std::vector<uint32>& indices);

  private:
    IndexBuffer(const IndexBuffer& buffer) = delete;
    IndexBuffer(const IndexBuffer&& buffer) = delete;
    IndexBuffer& operator=(const IndexBuffer& buffer) = delete;
    IndexBuffer& operator=(const IndexBuffer&& buffer) = delete;

  private:
    uint32 _ibo;
    uint32 _indexCount;
  };
}